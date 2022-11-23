#pragma warning (disable : 4047 4024 )
#include "MemoryEx.h"

NTSTATUS Memory::KernelReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;

	return MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(), TargetAddress, Size, KernelMode, reinterpret_cast<PSIZE_T>(&Bytes));
}

NTSTATUS Memory::KernelWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;

	return MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process, TargetAddress, Size, KernelMode, reinterpret_cast<PSIZE_T>(&Bytes));
}

PVOID Memory::VirtToPhy(PVOID Va)
{
	return (PVOID)MmGetPhysicalAddress(Va).QuadPart;
}

PVOID Memory::PhyToVirt(PVOID Pa)
{
	PHYSICAL_ADDRESS PhysicalAddr;
	PhysicalAddr.QuadPart = (ULONGLONG)Pa;

	return MmGetVirtualForPhysical(PhysicalAddr);
}

BOOLEAN Memory::AllocVmxonRegion(PVM_STATE pState)
{
    // at IRQL > DISPATCH_LEVEL memory allocation routines don't work
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
        KeRaiseIrqlToDpcLevel();

    PHYSICAL_ADDRESS PhysicalMax = { 0 };
    PhysicalMax.QuadPart = MAXULONG64;

    int VMXONSize = 2 * VMXON_SIZE;
    BYTE* Buffer = (BYTE*)MmAllocateContiguousMemory(VMXONSize + ALIGNMENT_PAGE_SIZE, PhysicalMax); // Allocating a 4-KByte Contigous Memory region

    PHYSICAL_ADDRESS Highest = { 0 };
    Highest.QuadPart = ~0;

    if (Buffer == NULL)
    {
        DbgMsg("Error : Couldn't Allocate Buffer for VMXON Region.");
        return FALSE; // ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    UINT64 PhysicalBuffer = (UINT64)VirtToPhy(Buffer);

    // zero-out memory
    RtlSecureZeroMemory(Buffer, VMXONSize + ALIGNMENT_PAGE_SIZE);
    UINT64 AlignedPhysicalBuffer = (ULONG_PTR)(PhysicalBuffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    UINT64 AlignedVirtualBuffer = (ULONG_PTR)(Buffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    DbgMsg("Virtual allocated buffer for VMXON at %p", Buffer);
    DbgMsg("Virtual aligned allocated buffer for VMXON at %llx", AlignedVirtualBuffer);
    DbgMsg("Aligned physical buffer allocated for VMXON at %llx", AlignedPhysicalBuffer);

    // get IA32_VMX_BASIC_MSR RevisionId

    IA32_VMX_BASIC_MSR basic = { 0 };

    basic.All = __readmsr(MSR_IA32_VMX_BASIC);

    DbgMsg("MSR_IA32_VMX_BASIC (MSR 0x480) Revision Identifier %x", basic.Fields.RevisionIdentifier);

    // Changing Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = basic.Fields.RevisionIdentifier;

    pState->VmxonRegion = AlignedPhysicalBuffer;

    return TRUE;
}

BOOLEAN Memory::AllocVmcsRegion(PVM_STATE pState)
{
    //
     // at IRQL > DISPATCH_LEVEL memory allocation routines don't work
     //
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
        KeRaiseIrqlToDpcLevel();

    PHYSICAL_ADDRESS PhysicalMax = { 0 };
    PhysicalMax.QuadPart = MAXULONG64;

    int    VMCSSize = 2 * VMCS_SIZE;
    BYTE* Buffer = (BYTE*)MmAllocateContiguousMemory(VMCSSize + ALIGNMENT_PAGE_SIZE, PhysicalMax); // Allocating a 4-KByte Contigous Memory region

    PHYSICAL_ADDRESS Highest = { 0 };
    Highest.QuadPart = ~0;

    // BYTE* Buffer = MmAllocateContiguousMemorySpecifyCache(VMXONSize + ALIGNMENT_PAGE_SIZE, Lowest, Highest, Lowest, MmNonCached);

    UINT64 PhysicalBuffer = (UINT64)VirtToPhy(Buffer);
    if (Buffer == NULL)
    {
        DbgMsg("Error : Couldn't Allocate Buffer for VMCS Region.");
        return FALSE; // ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    // zero-out memory
    RtlSecureZeroMemory(Buffer, VMCSSize + ALIGNMENT_PAGE_SIZE);
    UINT64 AlignedPhysicalBuffer = (ULONG_PTR)(PhysicalBuffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    UINT64 AlignedVirtualBuffer = (ULONG_PTR)(Buffer + ALIGNMENT_PAGE_SIZE - 1) & ~(ALIGNMENT_PAGE_SIZE - 1);

    DbgMsg("[*] Virtual allocated buffer for VMCS at %p", Buffer);
    DbgMsg("[*] Virtual aligned allocated buffer for VMCS at %llx", AlignedVirtualBuffer);
    DbgMsg("[*] Aligned physical buffer allocated for VMCS at %llx", AlignedPhysicalBuffer);

    // get IA32_VMX_BASIC_MSR RevisionId

    IA32_VMX_BASIC_MSR basic = { 0 };

    basic.All = __readmsr(MSR_IA32_VMX_BASIC);

    DbgMsg("[*] MSR_IA32_VMX_BASIC (MSR 0x480) Revision Identifier %x", basic.Fields.RevisionIdentifier);

    // Changing Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = basic.Fields.RevisionIdentifier;

    pState->VmcsRegion = AlignedPhysicalBuffer;

    return TRUE;
}
