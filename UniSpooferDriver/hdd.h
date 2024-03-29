#pragma once

#include "utils.h"
#include "data.h"

#pragma warning (disable:4702)

#define MAX_HDDS 10
#define SERIAL_MAX_LENGTH 15
#define SERIAL "MyDummySerial"

typedef char BYTE;

typedef struct _VendorInfo
{
	char pad_0x0000[0x8];
	char Info[64];
} VendorInfo;

typedef struct _HDD_EXTENSION
{
	char pad_0x0000[0x68];
	VendorInfo* pVendorInfo;
	char pad_0x0068[0x8];
	char* pHDDSerial;
	char pad_0x0078[0x30];
} HDD_EXTENSION, * PHDD_EXTENSION;

typedef NTSTATUS(__fastcall* DISK_FAIL_PREDICTION)(PVOID device_extension, BYTE enable);

typedef

NTSTATUS

(*pfnObReferenceObjectByName)(

	__in PUNICODE_STRING ObjectName,

	__in ULONG Attributes,

	__in_opt PACCESS_STATE AccessState,

	__in_opt ACCESS_MASK DesiredAccess,

	__in POBJECT_TYPE ObjectType,

	__in KPROCESSOR_MODE AccessMode,

	__inout_opt PVOID ParseContext,

	__out PVOID* Object

	);

void SpoofHDD();
void DisableAndSpoofSMART();
