#pragma once
#include <ntifs.h>
#include <ntdef.h>

//TYPE DEF DEFINITIONS
typedef int BOOL;

typedef struct Globals {
	static WCHAR UniHideKeysPath[512];
	static WCHAR CurrentDriverName[64];
	static bool IsInitialized;

	static void Init();
} globals;
