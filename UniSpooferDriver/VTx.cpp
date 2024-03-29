#include "VTx.h"

#include <intrin.h>

bool VTx::Init()
{
    DbgMsg("[VMX] Initializing VMX...");

    if (!IsVmxSupported())
    {
        DbgMsg("[VMX] VMX is not supported in this machine !");
        return FALSE;
    }

    ULONG ulMaxProc = KeQueryActiveProcessorCount(0);

    KAFFINITY AffinityMask;
    for (size_t i = 0; i < ulMaxProc; i++)
    {
        AffinityMask = 2 << i;
        Globals::Hyperv::ulProcessorMask |= AffinityMask;

        KeSetSystemAffinityThread(AffinityMask);

        //
        // Enabling VMX Operation
        //
        EnableVmx();
        if (!IsVmxEnabled()) {
            DbgMsg("[VMX] Error: could not enable VMX for processor: %llx", i);
            goto _error;
        }

        DbgMsg("[VMX] VMX Operation Enabled for logical processor: %llx", i);

        PVM_STATE pState = (PVM_STATE)cpp::kMalloc(sizeof(VM_STATE));
        Globals::Hyperv::vGuestStates.Append(pState);

        AllocVmxonRegion(pState);
        AllocVmcsRegion(pState);
        
        if (!pState->pVmcsRegion || !pState->pVmxonRegion) {
            DbgMsg("[VMX] Failed to allocate region for logical processor: %llx! Aborting initialization...", i);
            goto _error;
        }

        continue;
    _error:
        Globals::Hyperv::ulProcessorMask &= ~AffinityMask;
    }

    DbgMsg("[VMX] Successfully initialized VMX");

    return TRUE;
}

bool VTx::IsIntelCPU()
{
    char pManufacturer[0x20] = { 0 };

    __cpuidex((int*)pManufacturer, 0, 0);

    return RtlCompareMemory(pManufacturer, "Genu", 4) == 0;
}

bool VTx::IsMsrLocked()
{
    IA32_FEATURE_CONTROL_MSR Control = { 0 };
    Control.All = __readmsr(MSR_IA32_FEATURE_CONTROL);

    if (Control.Fields.Lock == 0)
    {
        return FALSE;
    }

    DbgMsg("[VMX] MSR is currently locked...");
    return TRUE;
}

bool VTx::VmxOn(PVOID pRegion)
{
    DbgMsg("[VMX] VMXON called...");
    
    char bStatus = __vmx_on((ULONGLONG*)pRegion);
    if (bStatus && bStatus != 2 /*Status 2 means that Vmx was already ON*/)
    {
        DbgMsg("[VMX] VMXON failed with status %x", bStatus);
        return FALSE;
    }
    return TRUE;
}

void VTx::VmxOff()
{
    DbgMsg("[VMX] Terminating VMX...");

    KAFFINITY AffinityMask;
    size_t i = 0;
    for (; i < sizeof(ULONG); i++)
    {
        AffinityMask = 2 << i;
        if (!(AffinityMask & Globals::Hyperv::ulProcessorMask))
            continue;
        KeSetSystemAffinityThread(AffinityMask);
        DbgMsg("[VMX] Calling VMXOFF for logical processor: %llx", i);

        __vmx_off();
        Globals::Hyperv::vGuestStates[i]->bVmxOn = false;

        cpp::kFree((PVOID)Memory::PhyToVirt(Globals::Hyperv::vGuestStates[i]->pVmxonRegion));
        cpp::kFree((PVOID)Memory::PhyToVirt(Globals::Hyperv::vGuestStates[i]->pVmcsRegion));
    }

    DbgMsg("[VMX] VMX Operation turned off successfully for %llx logical processors", i);
}

void VTx::Vmptrst()
{
    DbgMsg("[VMX] VMPTRST called...");
    
    PHYSICAL_ADDRESS vmcspa;
    vmcspa.QuadPart = 0;
    __vmx_vmptrst((unsigned __int64*)&vmcspa);

    DbgMsg("[VMX] VMPTRST %llx\n", vmcspa);
}

bool VTx::VmClear(PVM_STATE pState)
{
    // Clear the state of the VMCS to inactive
    int status = __vmx_vmclear(&pState->pVmcsRegion);

    DbgMsg("[VMX] VMCLEAR Status is : %d", status);
    if (status)
    {
        // Otherwise, terminate the VMX
        DbgMsg("[VMX] VMCLEAR failed with status %d", status);
        __vmx_off();
        pState->bVmxOn = false;
        return FALSE;
    }
    return TRUE;
}

bool VTx::VmPtrld(PVM_STATE pState)
{
    int status = __vmx_vmptrld(&pState->pVmcsRegion);
    if (status)
    {
        DbgMsg("[VMX] VMCS failed with status %d", status);
        return FALSE;
    }
    return TRUE;
}

void VTx::VmLaunch(ULONG ulProcessor, PEPTP pEpt)
{
    DbgMsg("[VMX] Launching VM for processor: 0x%x", ulProcessor);

    KAFFINITY kAffinity;
    kAffinity = 2 << ulProcessor;
    KeSetSystemAffinityThread(kAffinity);

    //
    // Allocate stack for the VM Exit Handler
    //
    UINT64 VMM_STACK_VA = (UINT64)cpp::kMalloc(VMM_STACK_SIZE);
    Globals::Hyperv::vGuestStates[ulProcessor]->pVmmStack = VMM_STACK_VA;

    if (Globals::Hyperv::vGuestStates[ulProcessor]->pVmmStack == NULL)
    {
        DbgMsg("[VMX] Error in allocating VMM Stack.");
        return;
    }
    RtlZeroMemory((PVOID)Globals::Hyperv::vGuestStates[ulProcessor]->pVmmStack, VMM_STACK_SIZE);

    //VMM_STACK_VA = (UINT64)cpp::kMalloc(VMM_STACK_SIZE);
    //Globals::Hyperv::vGuestStates[ulProcessor]->pGuestStack = VMM_STACK_VA;
    //
    //if (Globals::Hyperv::vGuestStates[ulProcessor]->pGuestStack == NULL)
    //{
    //    DbgMsg("[VMX] Error in allocating Guest Stack.");
    //    return;
    //}
    //RtlZeroMemory((PVOID)Globals::Hyperv::vGuestStates[ulProcessor]->pGuestStack, VMM_STACK_SIZE);

    //
    // Allocate memory for MSRBitMap
    //
    Globals::Hyperv::vGuestStates[ulProcessor]->vaMsrBitmap = (UINT64)cpp::kMallocNonCached(PAGE_SIZE); // should be aligned
    if (Globals::Hyperv::vGuestStates[ulProcessor]->vaMsrBitmap == NULL)
    {
        DbgMsg("[VMX] Error in allocating MSRBitMap.");
        return;
    }
    RtlZeroMemory((PVOID)Globals::Hyperv::vGuestStates[ulProcessor]->vaMsrBitmap, PAGE_SIZE);
    Globals::Hyperv::vGuestStates[ulProcessor]->paMsrBitmapPhysical = (UINT64)Memory::VirtToPhy((PVOID)Globals::Hyperv::vGuestStates[ulProcessor]->vaMsrBitmap);

    //
    // Clear the VMCS State
    //

    if (!VmClear(Globals::Hyperv::vGuestStates[ulProcessor])) {
        goto _error;
    }

    //
    // Load VMCS (Set the Current VMCS)
    //
    if (!VmPtrld(Globals::Hyperv::vGuestStates[ulProcessor])) {
        goto _error;
    }

    VmcsSetup(Globals::Hyperv::vGuestStates[ulProcessor], pEpt);

    DbgMsg("[VMX] Calling VMLAUNCH...");
    VmxSaveAndLaunch(Globals::Hyperv::vGuestStates[ulProcessor]->ulGuestRsp, Globals::Hyperv::vGuestStates[ulProcessor]->ulGuestRbp);

    //
    // VMLAUNCH will return here if a breaking VMEXIT case occurs
    //

    if (IsVmxEnabled() && Globals::Hyperv::vGuestStates[ulProcessor]->bVmxOn) {
        ULONG64 ErrorCode = 0;
        __vmx_vmread(VM_INSTRUCTION_ERROR, &ErrorCode);
        __vmx_off();
        Globals::Hyperv::vGuestStates[ulProcessor]->bVmxOn = false;
        DbgMsg("[VMX] VMLAUNCH Error: 0x%llx", ErrorCode);
    }

    return;

_error:
    DbgMsg("[VMX] There was an error starting the VM on logical processor: 0x%x", ulProcessor);
}

bool VTx::VmcsSetup(PVM_STATE pState, PEPTP pEpt)
{
    DbgMsg("[VMX] Setupping VMCS...");

    char bResult = 0;

    bResult |= __vmx_vmwrite(HOST_ES_SELECTOR, GetEs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_CS_SELECTOR, GetCs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_SS_SELECTOR, GetSs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_DS_SELECTOR, GetDs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_FS_SELECTOR, GetFs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_GS_SELECTOR, GetGs() & 0xF8);
    bResult |= __vmx_vmwrite(HOST_TR_SELECTOR, GetTr() & 0xF8);

    // Setting the link pointer to the required value for 4KB VMCS
    // Required for hypervisors that want to implement nested virtualization
    bResult |= __vmx_vmwrite(VMCS_LINK_POINTER, ~0ULL);

    // Configuring Guest IA32_DEBUGCTL MSR, used for debugging purposes
    bResult |= __vmx_vmwrite(GUEST_IA32_DEBUGCTL, __readmsr(MSR_IA32_DEBUGCTL) & 0xFFFFFFFF);
    bResult |= __vmx_vmwrite(GUEST_IA32_DEBUGCTL_HIGH, __readmsr(MSR_IA32_DEBUGCTL) >> 32);

    // Time-stamp counter offset
    bResult |= __vmx_vmwrite(TSC_OFFSET, 0);
    bResult |= __vmx_vmwrite(TSC_OFFSET_HIGH, 0);

    bResult |= __vmx_vmwrite(PAGE_FAULT_ERROR_CODE_MASK, 0);
    bResult |= __vmx_vmwrite(PAGE_FAULT_ERROR_CODE_MATCH, 0);

    bResult |= __vmx_vmwrite(VM_EXIT_MSR_STORE_COUNT, 0);
    bResult |= __vmx_vmwrite(VM_EXIT_MSR_LOAD_COUNT, 0);

    bResult |= __vmx_vmwrite(VM_ENTRY_MSR_LOAD_COUNT, 0);
    bResult |= __vmx_vmwrite(VM_ENTRY_INTR_INFO_FIELD, 0);

    PVOID pGdt = (PVOID)GetGdtBase();

    FillGuestData(pGdt, ES, GetEs());
    FillGuestData(pGdt, CS, GetCs());
    FillGuestData(pGdt, SS, GetSs());
    FillGuestData(pGdt, DS, GetDs());
    FillGuestData(pGdt, FS, GetFs());
    FillGuestData(pGdt, GS, GetGs());
    FillGuestData(pGdt, LDTR, GetLdtr());
    FillGuestData(pGdt, TR, GetTr());

    // FS holds the TLS whilst GS usually holds system structures (in ring 0)
    bResult |= __vmx_vmwrite(GUEST_FS_BASE, __readmsr(MSR_FS_BASE));
    bResult |= __vmx_vmwrite(GUEST_GS_BASE, __readmsr(MSR_GS_BASE));

    bResult |= __vmx_vmwrite(GUEST_INTERRUPTIBILITY_INFO, 0);
    bResult |= __vmx_vmwrite(GUEST_ACTIVITY_STATE, 0);             //Active state 

    //CPU_BASED_CTL2_ENABLE_EPT is used for hypervisors that want to enable "nested page table"-like behavior
    bResult |= __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, SetMsrCtl(CPU_BASED_HLT_EXITING | CPU_BASED_ACTIVATE_SECONDARY_CONTROLS, MSR_IA32_VMX_PROCBASED_CTLS));
    bResult |= __vmx_vmwrite(SECONDARY_VM_EXEC_CONTROL, SetMsrCtl(CPU_BASED_CTL2_RDTSCP /* | CPU_BASED_CTL2_ENABLE_EPT*/, MSR_IA32_VMX_PROCBASED_CTLS2));

    bResult |= __vmx_vmwrite(PIN_BASED_VM_EXEC_CONTROL, SetMsrCtl(0, MSR_IA32_VMX_PINBASED_CTLS));
    bResult |= __vmx_vmwrite(VM_EXIT_CONTROLS, SetMsrCtl(VM_EXIT_IA32E_MODE | VM_EXIT_ACK_INTR_ON_EXIT, MSR_IA32_VMX_EXIT_CTLS));
    bResult |= __vmx_vmwrite(VM_ENTRY_CONTROLS, SetMsrCtl(VM_ENTRY_IA32E_MODE, MSR_IA32_VMX_ENTRY_CTLS));

    bResult |= __vmx_vmwrite(CR3_TARGET_COUNT, 0);
    bResult |= __vmx_vmwrite(CR3_TARGET_VALUE0, 0);
    bResult |= __vmx_vmwrite(CR3_TARGET_VALUE1, 0);
    bResult |= __vmx_vmwrite(CR3_TARGET_VALUE2, 0);
    bResult |= __vmx_vmwrite(CR3_TARGET_VALUE3, 0);

    //Setup cr for GUEST
    bResult |= __vmx_vmwrite(GUEST_CR0, __readcr0());
    bResult |= __vmx_vmwrite(GUEST_CR3, __readcr3());
    bResult |= __vmx_vmwrite(GUEST_CR4, __readcr4());

    bResult |= __vmx_vmwrite(GUEST_DR7, 0x400);

    //Setup cr for HOST
    bResult |= __vmx_vmwrite(HOST_CR0, __readcr0());
    bResult |= __vmx_vmwrite(HOST_CR3, __readcr3());
    bResult |= __vmx_vmwrite(HOST_CR4, __readcr4());

    //It's generally a bad idea to use the same IDT and/or GDT for host and guest, since it can lead to out-of-VM break
    bResult |= __vmx_vmwrite(GUEST_GDTR_BASE, GetGdtBase());
    bResult |= __vmx_vmwrite(GUEST_IDTR_BASE, GetIdtBase());
    bResult |= __vmx_vmwrite(GUEST_GDTR_LIMIT, GetGdtLimit());
    bResult |= __vmx_vmwrite(GUEST_IDTR_LIMIT, GetIdtLimit());

    bResult |= __vmx_vmwrite(GUEST_RFLAGS, GetRflags());

    //These should be setup only if one wants to enable usage of SYSENTER
    bResult |= __vmx_vmwrite(GUEST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    bResult |= __vmx_vmwrite(GUEST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    bResult |= __vmx_vmwrite(GUEST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));

    SEG_SELECTOR SegmentSelector = { 0 };
    GetSegDesc(&SegmentSelector, GetTr(), (PUCHAR)GetGdtBase());
    bResult |= __vmx_vmwrite(HOST_TR_BASE, SegmentSelector.BASE);

    bResult |= __vmx_vmwrite(HOST_FS_BASE, __readmsr(MSR_FS_BASE));
    bResult |= __vmx_vmwrite(HOST_GS_BASE, __readmsr(MSR_GS_BASE));

    bResult |= __vmx_vmwrite(HOST_GDTR_BASE, GetGdtBase());
    bResult |= __vmx_vmwrite(HOST_IDTR_BASE, GetIdtBase());

    bResult |= __vmx_vmwrite(HOST_IA32_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    bResult |= __vmx_vmwrite(HOST_IA32_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    bResult |= __vmx_vmwrite(HOST_IA32_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));

    bResult |= __vmx_vmwrite(GUEST_RSP, (ULONG64)pState->pGuestMem);
    bResult |= __vmx_vmwrite(GUEST_RIP, (ULONG64)pState->pGuestMem);
    bResult |= __vmx_vmwrite(HOST_RSP, ((ULONG64)pState->pVmmStack + VMM_STACK_SIZE - 1));
    bResult |= __vmx_vmwrite(HOST_RIP, (ULONG64)VmExitWrapper);

    if (bResult) {
        DbgMsg("[VMX] Error: could not correctly initialize VMCS!");
        return false;
    }

    DbgMsg("[VMX] Successfully setup VMCS");
    return true;
}

void VTx::VmExitHandler(PGUEST_REGS pGuestRegs)
{
    ULONG ExitReason = 0;
    __vmx_vmread(VM_EXIT_REASON, (size_t*)&ExitReason);
    ExitReason &= 0xffff;

    ULONG ExitQualification = 0;
    __vmx_vmread(EXIT_QUALIFICATION, (size_t*)&ExitQualification);

    DbgMsg("[VMEXIT] VM_EXIT_REASON 0x%x", ExitReason);
    DbgMsg("[VMEXIT] EXIT_QUALIFICATION 0x%x", ExitQualification);

    switch (ExitReason)
    {
        //
        // 25.1.2  Instructions That Cause VM Exits Unconditionally
        // The following instructions cause VM exits when they are executed in VMX non-root operation: CPUID, GETSEC,
        // INVD, and XSETBV. This is also true of instructions introduced with VMX, which include: INVEPT, INVVPID,
        // VMCALL, VMCLEAR, VMLAUNCH, VMPTRLD, VMPTRST, VMRESUME, VMXOFF, and VMXON.
        //
    case EXIT_REASON_VMCLEAR:
    case EXIT_REASON_VMPTRLD:
    case EXIT_REASON_VMPTRST:
    case EXIT_REASON_VMREAD:
    case EXIT_REASON_VMRESUME:
    case EXIT_REASON_VMWRITE:
    case EXIT_REASON_VMXOFF:
    case EXIT_REASON_VMXON:
    case EXIT_REASON_VMLAUNCH:
    {
        break;
    }
    case EXIT_REASON_HLT:
    {
        DbgMsg("[VMEXIT] Execution of HLT detected...");

        //
        // that's enough for now ;)
        //

        break;
    }
    case EXIT_REASON_EXCEPTION_NMI:
    {
        break;
    }
    case EXIT_REASON_CPUID:
    {
        break;
    }
    case EXIT_REASON_INVD:
    {
        break;
    }
    case EXIT_REASON_VMCALL:
    {
        break;
    }
    case EXIT_REASON_CR_ACCESS:
    {
        break;
    }
    case EXIT_REASON_MSR_READ:
    {
        break;
    }
    case EXIT_REASON_MSR_WRITE:
    {
        break;
    }
    case EXIT_REASON_EPT_VIOLATION:
    {
        break;
    }
    case EXIT_REASON_INVALID_GUEST_STATE:
    {
        DbgMsg("[VMEXIT] Error: an invalid guest state was detected!");
        Checks::CheckGuestVmcsFieldsForVmEntry();
        break;
    }
    default:
    {
        break;
    }
    }

    Globals::Hyperv::vGuestStates[0]->bVmxOn = false;
    VmxRestore(Globals::Hyperv::vGuestStates[0]->ulGuestRsp, Globals::Hyperv::vGuestStates[0]->ulGuestRbp);
}

void VTx::VmResumeExec()
{
    DbgMsg("[VMX] Resuming execution...");

    __vmx_vmresume();

    // if VMRESUME succeeds will never be here !

    ULONG64 ErrorCode = 0;
    __vmx_vmread(VM_INSTRUCTION_ERROR, &ErrorCode);
    VmxOff();

    DbgMsg("[VMX] VMRESUME Error : 0x%llx", ErrorCode);
}

void VTx::MoveRip()
{
    DbgMsg("[VMX] Moving RIP from current instruction...");

    PVOID ResumeRIP = NULL;
    size_t CurrentRIP = NULL;
    size_t ExitInstructionLength = 0;

    __vmx_vmread(GUEST_RIP, &CurrentRIP);
    __vmx_vmread(VM_EXIT_INSTRUCTION_LEN, &ExitInstructionLength);

    ResumeRIP = (PCHAR)CurrentRIP + ExitInstructionLength;

    __vmx_vmwrite(GUEST_RIP, (ULONG64)ResumeRIP);
}

bool VTx::AllocVmxonRegion(PVM_STATE pState)
{
    DbgMsg("[MEM] Allocating VMXON Region...");

    int VMXONSize = 2 * VMXON_SIZE;
    BYTE* Buffer = (BYTE*)cpp::kMallocContinuous(VMXONSize + ALIGNMENT_PAGE_SIZE); // Allocating a 4-KByte Contigous Memory region

    PHYSICAL_ADDRESS Highest = { 0 };
    Highest.QuadPart = ~0;

    if (Buffer == NULL)
    {
        DbgMsg("[MEM] Error : Couldn't Allocate Buffer for VMXON Region.");
        return FALSE; // ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    UINT64 PhysicalBuffer = (UINT64)Memory::VirtToPhy(Buffer);

    // zero-out memory
    RtlSecureZeroMemory(Buffer, VMXONSize + ALIGNMENT_PAGE_SIZE);
    UINT64 AlignedPhysicalBuffer = (ULONG_PTR)(PhysicalBuffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    UINT64 AlignedVirtualBuffer = (ULONG_PTR)(Buffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    DbgMsg("[MEM] Virtual allocated buffer for VMXON at %p", Buffer);
    DbgMsg("[MEM] Virtual aligned allocated buffer for VMXON at %llx", AlignedVirtualBuffer);
    DbgMsg("[MEM] Aligned physical buffer allocated for VMXON at %llx", AlignedPhysicalBuffer);

    // get IA32_VMX_BASIC_MSR RevisionId

    IA32_VMX_BASIC_MSR basic = { 0 };

    basic.All = __readmsr(MSR_IA32_VMX_BASIC);

    DbgMsg("[MEM] MSR_IA32_VMX_BASIC (MSR 0x480) Revision Identifier %x", basic.Fields.RevisionIdentifier);

    // Changing Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = basic.Fields.RevisionIdentifier;

    pState->pVmxonRegion = AlignedPhysicalBuffer;

    char bStatus = VmxOn(&AlignedPhysicalBuffer);
    if (!bStatus && bStatus != 2 /*Status 2 means that Vmx was already ON*/) {
        return FALSE;
    }

    pState->bVmxOn = true;
    return TRUE;
}

bool VTx::AllocVmcsRegion(PVM_STATE pState)
{
    DbgMsg("[MEM] Allocating VMCS Region...");

    int    VMCSSize = 2 * VMCS_SIZE;
    BYTE* Buffer = (BYTE*)cpp::kMallocContinuous(VMCSSize + ALIGNMENT_PAGE_SIZE); // Allocating a 4-KByte Contigous Memory region

    PHYSICAL_ADDRESS Highest = { 0 };
    Highest.QuadPart = ~0;

    UINT64 PhysicalBuffer = (UINT64)Memory::VirtToPhy(Buffer);
    if (Buffer == NULL)
    {
        DbgMsg("Error : Couldn't Allocate Buffer for VMCS Region.");
        return FALSE;
    }
    // zero-out memory
    RtlSecureZeroMemory(Buffer, VMCSSize + ALIGNMENT_PAGE_SIZE);
    UINT64 AlignedPhysicalBuffer = (ULONG_PTR)(PhysicalBuffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    UINT64 AlignedVirtualBuffer = (ULONG_PTR)(Buffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    DbgMsg("[MEM] Virtual allocated buffer for VMCS at %p", Buffer);
    DbgMsg("[MEM] Virtual aligned allocated buffer for VMCS at %llx", AlignedVirtualBuffer);
    DbgMsg("[MEM] Aligned physical buffer allocated for VMCS at %llx", AlignedPhysicalBuffer);

    // get IA32_VMX_BASIC_MSR RevisionId

    IA32_VMX_BASIC_MSR basic = { 0 };

    basic.All = __readmsr(MSR_IA32_VMX_BASIC);

    DbgMsg("[MEM] MSR_IA32_VMX_BASIC (MSR 0x480) Revision Identifier %x", basic.Fields.RevisionIdentifier);

    // Changing Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = basic.Fields.RevisionIdentifier;

    pState->pVmcsRegion = AlignedPhysicalBuffer;

    return TRUE;
}

void VTx::FillGuestData(PVOID pGdt, ULONG ulSegReg, USHORT usSelector)
{
    SEG_SELECTOR SegmentSelector = { 0 };
    ULONG            AccessRights;
    char            bResult = 0;

    GetSegDesc(&SegmentSelector, usSelector, pGdt);
    AccessRights = ((PUCHAR)&SegmentSelector.ATTRIBUTES)[0] + (((PUCHAR)&SegmentSelector.ATTRIBUTES)[1] << 12);

    if (!usSelector)
        AccessRights |= 0x10000;

    bResult |= __vmx_vmwrite(GUEST_ES_SELECTOR + ulSegReg * 2, usSelector);
    bResult |= __vmx_vmwrite(GUEST_ES_LIMIT + ulSegReg * 2, SegmentSelector.LIMIT);
    bResult |= __vmx_vmwrite(GUEST_ES_AR_BYTES + ulSegReg * 2, AccessRights);
    bResult |= __vmx_vmwrite(GUEST_ES_BASE + ulSegReg * 2, SegmentSelector.BASE);

    if (bResult) {
        DbgMsg("[VMX] Error: could not correctly fill guest data!");
    }
}

bool VTx::GetSegDesc(PSEG_SELECTOR pSegSel, USHORT usSelector, PVOID pGdt)
{
    PSEG_DESCRIPTOR SegDesc;

    if (!pSegSel)
        return FALSE;

    if (usSelector & 0x4)
    {
        return FALSE;
    }

    SegDesc = (PSEG_DESCRIPTOR)((PUCHAR)pGdt + (usSelector & ~0x7));

    pSegSel->SEL = usSelector;
    pSegSel->BASE = SegDesc->BASE0 | SegDesc->BASE1 << 16 | SegDesc->BASE2 << 24;
    pSegSel->LIMIT = SegDesc->LIMIT0 | (SegDesc->LIMIT1ATTR1 & 0xf) << 16;
    pSegSel->ATTRIBUTES.UCHARs = SegDesc->ATTR0 | (SegDesc->LIMIT1ATTR1 & 0xf0) << 4;

    if (!(SegDesc->ATTR0 & 0x10))
    { // LA_ACCESSED
        ULONG64 Tmp;
        // this is a TSS or callgate etc, save the base high part
        Tmp = (*(PULONG64)((PUCHAR)SegDesc + 8));
        pSegSel->BASE = (pSegSel->BASE & 0xffffffff) | (Tmp << 32);
    }

    if (pSegSel->ATTRIBUTES.Fields.G)
    {
        // 4096-bit granularity is enabled for this segment, scale the limit
        pSegSel->LIMIT = (pSegSel->LIMIT << 12) + 0xfff;
    }

    return TRUE;
}

ULONG VTx::SetMsrCtl(ULONG ulCtl, ULONG ulMsr)
{
    MSR MsrValue = { 0 };

    MsrValue.Content = __readmsr(ulMsr);
    ulCtl &= MsrValue.High; /* bit == 0 in high word ==> must be zero */
    ulCtl |= MsrValue.Low;  /* bit == 1 in low word  ==> must be one  */
    return ulCtl;
}

