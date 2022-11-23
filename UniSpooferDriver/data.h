#pragma once
#include <ntifs.h>
#include <ntdef.h>

#include "VectorEx.h"

typedef struct _VM_STATE
{
	UINT64 VmxonRegion; // VMXON region
	UINT64 VmcsRegion;  // VMCS region
} VM_STATE, * PVM_STATE;

//TYPE DEF DEFINITIONS
typedef char BOOL;
typedef char BYTE;

typedef struct Globals {
	static WCHAR UniHideKeysPath[512];
	static WCHAR CurrentDriverName[64];
	static bool IsInitialized;

	//Hypervisor related
	static vector<PVM_STATE> vGuestStates;
	static ULONG ulProcessorCount;

	static void Init();
} globals;
