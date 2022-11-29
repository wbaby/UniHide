#include "EPT.h"

UINT64 EPT::InitEptp(ULONG ulProcessor)
{
    DbgMsg("[EPT] Initializing EPT pointer...");

	PAGED_CODE();
    PEPTP EPTPointer = (PEPTP)kMalloc(PAGE_SIZE);
    PEPT_PML4E EptPml4 = nullptr;
    PEPT_PDE EptPdpt = nullptr;
    PEPT_PDE EptPd = nullptr;
    PEPT_PTE EptPt = nullptr;

    if (!EPTPointer)
    {
        goto _cleanup;
    }
    RtlZeroMemory(EPTPointer, PAGE_SIZE);

    EptPml4 = (PEPT_PML4E)kMalloc(PAGE_SIZE);
    if (!EptPml4)
    {
        goto _cleanup;
    }
    RtlZeroMemory(EptPml4, PAGE_SIZE);

    EptPdpt = (PEPT_PDE)kMalloc(PAGE_SIZE);
    if (!EptPdpt)
    {
        goto _cleanup;
    }
    RtlZeroMemory(EptPdpt, PAGE_SIZE);

    EptPd = (PEPT_PDE)kMalloc(PAGE_SIZE);
    if (!EptPd)
    {
        goto _cleanup;
    }
    RtlZeroMemory(EptPdpt, PAGE_SIZE);

    EptPt = (PEPT_PTE)kMalloc(PAGE_SIZE);

    if (!EptPt)
    {
        goto _cleanup;
    }
    RtlZeroMemory(EptPt, PAGE_SIZE);

    //
    // Setup PT by allocating two pages Continuously
    // We allocate two pages because we need 1 page for our RIP to start and 1 page for RSP 1 + 1 = 2
    //
    const int PagesToAllocate = 100;
    UINT64 GuestMemory = (UINT64)kMalloc(PagesToAllocate * PAGE_SIZE);
    globals::vGuestStates[ulProcessor]->pGuestMem = GuestMemory;
    RtlZeroMemory((PVOID)GuestMemory, PagesToAllocate * PAGE_SIZE);

    for (size_t i = 0; i < PagesToAllocate; i++)
    {
        EptPt[i].Fields.AccessedFlag = 0;
        EptPt[i].Fields.DirtyFlag = 0;
        EptPt[i].Fields.EPTMemoryType = 6;
        EptPt[i].Fields.Execute = 1;
        EptPt[i].Fields.ExecuteForUserMode = 0;
        EptPt[i].Fields.IgnorePAT = 0;
        EptPt[i].Fields.PhysicalAddress = ((UINT64)Memory::VirtToPhy((PVOID)((GuestMemory + (i * PAGE_SIZE)))) / PAGE_SIZE);
        EptPt[i].Fields.Read = 1;
        EptPt[i].Fields.SuppressVE = 0;
        EptPt[i].Fields.Write = 1;
    }

    //
    // Setting up PDE
    //
    EptPd->Fields.Accessed = 0;
    EptPd->Fields.Execute = 1;
    EptPd->Fields.ExecuteForUserMode = 0;
    EptPd->Fields.Ignored1 = 0;
    EptPd->Fields.Ignored2 = 0;
    EptPd->Fields.Ignored3 = 0;
    EptPd->Fields.PhysicalAddress = ((UINT64)Memory::VirtToPhy((PVOID)((UINT64)(EptPt))) / PAGE_SIZE);
    EptPd->Fields.Read = 1;
    EptPd->Fields.Reserved1 = 0;
    EptPd->Fields.Reserved2 = 0;
    EptPd->Fields.Write = 1;
    //
    // Setting up PDPTE
    //
    EptPdpt->Fields.Accessed = 0;
    EptPdpt->Fields.Execute = 1;
    EptPdpt->Fields.ExecuteForUserMode = 0;
    EptPdpt->Fields.Ignored1 = 0;
    EptPdpt->Fields.Ignored2 = 0;
    EptPdpt->Fields.Ignored3 = 0;
    EptPdpt->Fields.PhysicalAddress = ((UINT64)Memory::VirtToPhy((PVOID)((UINT64)(EptPd))) / PAGE_SIZE);
    EptPdpt->Fields.Read = 1;
    EptPdpt->Fields.Reserved1 = 0;
    EptPdpt->Fields.Reserved2 = 0;
    EptPdpt->Fields.Write = 1;
    //
    // Setting up PML4E
    //
    EptPml4->Fields.Accessed = 0;
    EptPml4->Fields.Execute = 1;
    EptPml4->Fields.ExecuteForUserMode = 0;
    EptPml4->Fields.Ignored1 = 0;
    EptPml4->Fields.Ignored2 = 0;
    EptPml4->Fields.Ignored3 = 0;
    EptPml4->Fields.PhysicalAddress = ((UINT64)Memory::VirtToPhy((PVOID)((UINT64)(EptPdpt))) / PAGE_SIZE);
    EptPml4->Fields.Read = 1;
    EptPml4->Fields.Reserved1 = 0;
    EptPml4->Fields.Reserved2 = 0;
    EptPml4->Fields.Write = 1;
    //
    // Setting up EPTP
    //
    EPTPointer->Fields.DirtyAndAceessEnabled = 1;
    EPTPointer->Fields.MemoryType = 6; // 6 = Write-back (WB)
    EPTPointer->Fields.PageWalkLength = 3; // 4 (tables walked) - 1 = 3
    EPTPointer->Fields.PML4Address = ((UINT64)Memory::VirtToPhy((PVOID)((UINT64)(EptPml4))) / PAGE_SIZE);
    EPTPointer->Fields.Reserved1 = 0;
    EPTPointer->Fields.Reserved2 = 0;

    DbgMsg("[EPT] Extended Page Table located at %p", EPTPointer);
    return (UINT64)EPTPointer;

_cleanup:
    if(!EptPd)
        kDelete(EptPd);
    if(!EptPdpt)
        kDelete(EptPdpt);
    if(!EptPml4)
        kDelete(EptPml4);
    if(!EPTPointer)
        kDelete(EPTPointer);

    return 0;
}
