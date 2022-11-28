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
