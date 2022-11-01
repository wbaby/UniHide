#pragma once
#include <ntifs.h>
#include <ntdef.h>

//TYPE DEF DEFINITIONS
typedef int BOOL;

typedef struct Globals {
	static ULONGLONG ModuleAddress;
	static PDEVICE_OBJECT pDeviceObject;
	static PUNICODE_STRING dev, dos;

	static void Init();
} globals;
