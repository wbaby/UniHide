//This file contains all the useful definitions that you would normally find in usermode
//but reimplemented for kernel mode c++ :)
#pragma warning (disable: 4100 4996)

#pragma once

#ifndef _WDMDDK_
#include <ntifs.h>
#endif

#include "macros.h"

namespace cpp {
    //allocate new takes a pool type and returns a ptr to that mem, without calling the constructor of the obj tho
    void* kMalloc(size_t size, POOL_TYPE pool_type = NonPagedPool, bool collect = true);
    void* kMalloc(size_t size, int sig, bool collect = true);

    void* kMallocContinuous(size_t size, bool collect = true);
    //placement new takes an already allocated memory pool and calls the constructor of the obj
    void kFree(void* pObj, bool collect = true);
    void kFreeContinuous(void* pObj, bool collect = true);
}

void* operator new(size_t /* ignored */, void* where);

template <typename T>
constexpr bool is_lvalue(T&) {
    return true;
}

template <typename T>
constexpr bool is_lvalue(T&&) {
    return false;
}
