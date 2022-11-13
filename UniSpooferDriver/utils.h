#pragma once
#include <ntifs.h>

#include "cpp.h"
#include "StringEx.h"

//Remember to deallocate string's buffer after you're done using the UNICODE_STRING returned from this function
UNICODE_STRING GetModuleNameFromPath(PUNICODE_STRING path);

template<typename T>
void RandString(T* out, size_t len) {

	static char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	ULONG seed = KeQueryTimeIncrement();

	if (out)
	{
		for (int n = 0; n <= len; n++)
		{
			int key = RtlRandomEx(&seed) % (int)(sizeof(charset) - 1);
			out[n] = (T)charset[key];
		}
		out[len] = (T)0;
	}
}

PVOID GetKernelAddress(PCHAR name);
