#include "cpp.h"
#include "collector.h"

void* kMalloc(size_t size, POOL_TYPE pool_type, bool collect)
{
    void* p;
#ifndef DRIVER_TAG
    p = ExAllocatePool(pool_type, size);
#else
    p = ExAllocatePoolWithTag(pool_type, size, DRIVER_TAG);
#endif

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d bytes\n", (int)size);
    }
    if (collect)
        Collector::Add(p);
    return p;
}

void* kMalloc(size_t size, int sig, bool collect)
{
    void* p;
    p = ExAllocatePoolWithTag(NonPagedPool, size, sig);

    if (p == nullptr) {
        DbgMsg("Failed to allocate %d bytes\n", (int)size);
    }
    if (collect)
        Collector::Add(p);
    return p;
}

void* operator new(size_t /* ignored */, void* where) { return where; };

void kDelete(void* pObj, bool collect)
{
    ExFreePool(pObj);

    if(collect)
        Collector::Remove(pObj);
}

