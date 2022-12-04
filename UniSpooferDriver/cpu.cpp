#include "cpu.h"

BOOLEAN HYPERV_HANDLER CPU::HandleCPUID(PGUEST_REGS pContext)
{
    INT32 CpuInfo[4];
    size_t Mode = 0;

    //
    // Check for the magic CPUID sequence, and check that it is coming from
    // Ring 0. Technically we could also check the RIP and see if this falls
    // in the expected function, but we may want to allow a separate "unload"
    // driver or code at some point
    //

    __vmx_vmread(GUEST_CS_SELECTOR, &Mode);
    Mode = Mode & RPL_MASK;

    if (pContext->rax == Globals::szEndFlag && Mode == DPL_SYSTEM)
    {
        return TRUE; // Indicates we have to turn off VMX
    }

    //
    // Otherwise, issue the CPUID to the logical processor based on the indexes
    // on the VP's GPRs
    //
    __cpuidex(CpuInfo, (INT32)pContext->rax, (INT32)pContext->rcx);

    //
    // Check if this was CPUID 1h, which is the features request
    //
    if (pContext->rax == 1)
    {
        //
        // Set the Hypervisor Present-bit in RCX, which Intel and AMD have both
        // reserved for this indication
        //
        CpuInfo[2] |= HYPERV_HYPERVISOR_PRESENT_BIT;
    }

    else if (pContext->rax == HYPERV_CPUID_INTERFACE)
    {
        //
        // Return our interface identifier
        //
        CpuInfo[0] = 'UnHd';
    }

    //
    // Copy the values from the logical processor registers into the VP GPRs
    //
    pContext->rax = CpuInfo[0];
    pContext->rbx = CpuInfo[1];
    pContext->rcx = CpuInfo[2];
    pContext->rdx = CpuInfo[3];

    return FALSE; // Indicates we don't have to turn off VMX
}
