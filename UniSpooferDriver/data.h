#pragma once
#include <ntifs.h>
#include <ntdef.h>

#include "VectorEx.h"
#include "events.h"

#pragma warning (disable:4458)

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
	BOOLEAN bVmxOn;				// Is in VMX operation
} VM_STATE, *PVM_STATE;

//TYPE DEF DEFINITIONS
typedef char BOOL;
typedef char BYTE;

namespace Globals {
	extern WCHAR UniHideKeysPath[];
	extern WCHAR CurrentDriverName[];
	extern bool IsInitialized;

	//Hypervisor related
	extern vector<PVM_STATE> vGuestStates;
	extern ULONG ulProcessorMask;

	//Logging
	extern EventLogger evLogger;

	//CPU
	extern size_t szEndFlag; //StopVirt in ASCII

	void Init(PDRIVER_OBJECT pDevice);
}
