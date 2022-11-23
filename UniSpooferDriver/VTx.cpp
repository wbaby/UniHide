#include "VTx.h"

#include <intrin.h>

bool VTx::Init()
{
    if (!IsVmxSupported())
    {
        DbgMsgLevels(DEBUG_VMX, "VMX is not supported in this machine !");
        return FALSE;
    }

    globals::ulProcessorCount = KeQueryActiveProcessorCount(0);

    KAFFINITY AffinityMask;
    for (size_t i = 0; i < globals::ulProcessorCount; i++)
    {
        AffinityMask = 2 << i;

        KeSetSystemAffinityThread(AffinityMask);

        //
        // Enabling VMX Operation
        //
        EnableVmx();

        DbgMsgLevels(DEBUG_VMX, "VMX Operation Enabled for logical processor: %llx", i);

        PVM_STATE pState = (PVM_STATE)kMalloc(sizeof(VM_STATE));
        globals::vGuestStates.Append(pState);

        Memory::AllocVmxonRegion(globals::vGuestStates[i]);
        Memory::AllocVmcsRegion(globals::vGuestStates[i]);
        
        if (!globals::vGuestStates[i]->VmcsRegion || !globals::vGuestStates[i]->VmxonRegion) {
            DbgMsgLevels(DEBUG_VMX, "Failed to allocate region for logical processor: %llx! Aborting initialization...", i);
            return FALSE;
        }
    }

    DbgMsgLevels(DEBUG_VMX, "Successfully initialized VMX");

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

    DbgMsgLevels(DEBUG_VMX, "MSR is currently locked...");
    return TRUE;
}

bool VTx::VmxOn(PVOID pRegion)
{
    int Status = __vmx_on((ULONGLONG*)pRegion);
    if (Status)
    {
        DbgMsgLevels(DEBUG_VMX, "VMXON failed with status %x\n", Status);
        return FALSE;
    }
    return TRUE;
}

bool VTx::Vmptrld(PVOID pRegion)
{
    int Status = __vmx_vmptrld((ULONGLONG*)pRegion);
    if (Status)
    {
        DbgMsgLevels(DEBUG_VMX, "VMCS failed with status %x", Status);
        return FALSE;
    }
    return TRUE;
}

void VTx::VmxOff()
{
    DbgMsgLevels(DEBUG_VMX, "Terminating VMX...");

    KAFFINITY AffinityMask;
    size_t i = 0;
    for (; i < globals::ulProcessorCount; i++)
    {
        AffinityMask = 2 << i;
        KeSetSystemAffinityThread(AffinityMask);
        DbgMsgLevels(DEBUG_VMX, "Calling VMXOFF for logical processor: %llx", i);

        __vmx_off();

        MmFreeContiguousMemory(Memory::PhyToVirt((PVOID)globals::vGuestStates[i]->VmxonRegion));
        MmFreeContiguousMemory(Memory::PhyToVirt((PVOID)globals::vGuestStates[i]->VmcsRegion));
    }

    DbgMsgLevels(DEBUG_VMX, "VMX Operation turned off successfully for %llx logical processors", i);
}
