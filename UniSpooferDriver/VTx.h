#pragma once

#pragma warning (disable:4267)

#include "macros.h"
#include "data.h"
#include "MemoryEx.h"

#define MSR_IA32_FEATURE_CONTROL    0x0000003a
#define MSR_IA32_VMX_BASIC          0x00000480
#define VMXON_SIZE                  4096
#define VMCS_SIZE                   4096
#define ALIGNMENT_PAGE_SIZE         4096

typedef union _IA32_FEATURE_CONTROL_MSR
{
    ULONG64 All;
    struct
    {
        ULONG64 Lock : 1;               // [0]
        ULONG64 EnableSMX : 1;          // [1]
        ULONG64 EnableVmxon : 1;        // [2]
        ULONG64 Reserved2 : 5;          // [3-7]
        ULONG64 EnableLocalSENTER : 7;  // [8-14]
        ULONG64 EnableGlobalSENTER : 1; // [15]
        ULONG64 Reserved3a : 16;        //
        ULONG64 Reserved3b : 32;        // [16-63]
    } Fields;
} IA32_FEATURE_CONTROL_MSR, * PIA32_FEATURE_CONTROL_MSR;

typedef union _IA32_VMX_BASIC_MSR
{
    ULONG64 All;
    struct
    {
        ULONG32 RevisionIdentifier : 31;  // [0-30]
        ULONG32 Reserved1 : 1;            // [31]
        ULONG32 RegionSize : 12;          // [32-43]
        ULONG32 RegionClear : 1;          // [44]
        ULONG32 Reserved2 : 3;            // [45-47]
        ULONG32 SupportedIA64 : 1;        // [48]
        ULONG32 SupportedDualMoniter : 1; // [49]
        ULONG32 MemoryType : 4;           // [50-53]
        ULONG32 VmExitReport : 1;         // [54]
        ULONG32 VmxCapabilityHint : 1;    // [55]
        ULONG32 Reserved3 : 8;            // [56-63]
    } Fields;
} IA32_VMX_BASIC_MSR, * PIA32_VMX_BASIC_MSR;

namespace VTx
{
    bool Init();
    bool IsIntelCPU();
    bool IsMsrLocked();
    extern "C" bool IsVmxSupported();
    extern "C" bool IsVmxEnabled();
    extern "C" bool EnableVmx();
    extern "C" bool DisableVmx();
    bool VmxOn(PVOID pRegion);
    bool Vmptrld(PVOID pRegion);
    void VmxOff();
};
