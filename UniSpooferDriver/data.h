#pragma once
#include <ntifs.h>
#include <ntdef.h>

#include "VectorEx.h"

typedef struct _VM_STATE
{
	UINT64 pVmxonRegion;		// VMXON region
	UINT64 pVmcsRegion;			// VMCS region
	UINT64 pEpt;				// Extended-Page-Table Pointer
	UINT64 pVmmStack;			// Stack for VMM in VM-Exit State
	UINT64 pGuestMem;			// Guest emulation of physical
	UINT64 vaMsrBitmap;			// MSR Bitmap Virtual Address
	UINT64 paMsrBitmapPhysical;	// MSR Bitmap Physical Address
	UINT64 ulGuestRsp;			// Guest saved rsp
	UINT64 ulGuestRbp;			// Guest saved rbp
} VM_STATE, *PVM_STATE;

//TYPE DEF DEFINITIONS
typedef char BOOL;
typedef char BYTE;

typedef struct Globals {
	static WCHAR UniHideKeysPath[512];
	static WCHAR CurrentDriverName[64];
	static bool IsInitialized;

	//Hypervisor related
	static vector<PVM_STATE> vGuestStates;
	static ULONG ulProcessorMask;

	static void Init();
} globals;
