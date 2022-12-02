#include "cpp.h"
#include "collector.h"

void* cpp::kMalloc(size_t size, POOL_TYPE pool_type, bool collect)
{
    void* p;
#ifndef DRIVER_TAG
    p = ExAllocatePool(pool_type, size);
#else
    p = ExAllocatePoolWithTag(pool_type, size, DRIVER_TAG);
#endif

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d bytes", (int)size);
    }
    if (collect)
        Collector::Add(p, NonPaged);
    return p;
}

void* cpp::kMalloc(size_t size, int sig, bool collect)
{
    void* p;
    p = ExAllocatePoolWithTag(NonPagedPool, size, sig);

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d bytes", (int)size);
    }
    if (collect)
        Collector::Add(p, NonPaged);
    return p;
}

void* cpp::kMallocContinuous(size_t size)
{
    void* p;
    PHYSICAL_ADDRESS PhysicalMax = { 0 };
    PhysicalMax.QuadPart = MAXULONG64;
    p = MmAllocateContiguousMemory(size, PhysicalMax);

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d continuous bytes", (int)size);
    }
    Collector::Add(p, Continuous);

    return p;
}

void* cpp::kMallocNonCached(size_t size)
{
    void* p;
    p = MmAllocateNonCachedMemory(size);

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d non cached bytes", (int)size);
    }
    Collector::Add(p, NonCached, size);

    return p;
}

void* operator new(size_t /* ignored */, void* where) { return where; };

void cpp::kFree(void* pObj, bool collect)
{
    Collector::Clean(pObj, collect);
}
