#pragma once
#include <ntifs.h>

#include "cpp.h"
#include "StringEx.h"

//Remember to deallocate string's buffer after you're done using the UNICODE_STRING returned from this function
UNICODE_STRING GetModuleNameFromPath(PUNICODE_STRING path);

void RandString(char* out, size_t len);
PVOID GetKernelAddress(PCHAR name);
