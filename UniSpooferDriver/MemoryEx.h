#pragma once

#include <ntifs.h>
#include <intrin.h>

#include "macros.h"
#include "VTx.h"
#include "data.h"

DRIVER_IMPORT_API NTSTATUS NTAPI MmCopyVirtualMemory(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

namespace Memory {
	NTSTATUS KernelReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);
	NTSTATUS KernelWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size);
	PVOID VirtToPhy(PVOID Va);
	PVOID PhyToVirt(PVOID Pa);
	BOOLEAN AllocVmxonRegion(PVM_STATE pState);
	BOOLEAN AllocVmcsRegion(PVM_STATE pState);
}
