#pragma once

#include "EPT.h"
#include "VTx.h"
#include "data.h"

#define RPL_MASK   3

//Ring definition
#define DPL_USER   3
#define DPL_SYSTEM 0

//CPUID
#define HYPERV_CPUID_VENDOR_AND_MAX_FUNCTIONS 0x40000000
#define HYPERV_CPUID_INTERFACE                0x40000001
#define HYPERV_CPUID_VERSION                  0x40000002
#define HYPERV_CPUID_FEATURES                 0x40000003
#define HYPERV_CPUID_ENLIGHTMENT_INFO         0x40000004
#define HYPERV_CPUID_IMPLEMENT_LIMITS         0x40000005

#define HYPERV_HYPERVISOR_PRESENT_BIT 0x80000000
#define HYPERV_CPUID_MIN              0x40000005
#define HYPERV_CPUID_MAX              0x4000ffff

namespace CPU {
	template<typename F>
	void RunOnCpu(ULONG ulCpu, PEPTP EPTP, F callback) {
        // Raise IRQL to avoid context switches
        KIRQL OldIrql;
        KeSetSystemAffinityThread((KAFFINITY)(1 << ulCpu));
        OldIrql = KeRaiseIrqlToDpcLevel();

        callback(ulCpu, EPTP);

        // Restore IRQL
        KeLowerIrql(OldIrql);
        KeRevertToUserAffinityThread();

        return TRUE;
	}

    BOOLEAN HYPERV_HANDLER HandleCPUID(PGUEST_REGS pContext);
}
