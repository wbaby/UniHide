#pragma once

#include "utils.h"
#include "data.h"

#pragma warning (disable:4702)

#define MAX_HDDS 10
#define SERIAL_MAX_LENGTH 15


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

void SpoofHDD();
