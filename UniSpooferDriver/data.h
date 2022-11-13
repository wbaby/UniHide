#pragma once
#include <ntifs.h>
#include <ntdef.h>

//TYPE DEF DEFINITIONS
typedef int BOOL;

typedef struct Globals {
	static WCHAR RURIKeyPath[512];

	static void Init();
} globals;
