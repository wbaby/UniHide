#include "hdd.h"

DECLARE_HANDLE(HKEY);
#define HKEY_LOCAL_MACHINE                  (( HKEY ) (ULONG_PTR)((LONG)0x80000002) )

typedef __int64(__fastcall* RaidUnitRegisterInterfaces)(PHDD_EXTENSION a1);
typedef NTSTATUS(__fastcall* DISK_FAIL_PREDICTION)(PVOID device_extension, BYTE enable);
RaidUnitRegisterInterfaces pRegDevInt = NULL;
DISK_FAIL_PREDICTION pDiskFailPrediction = NULL;

RTL_QUERY_REGISTRY_ROUTINE RtlQueryQWORD;

_Use_decl_annotations_
NTSTATUS RtlQueryQWORD(PWSTR ValueName, ULONG ValueType, PVOID ValueData, ULONG ValueLength, PVOID Context, PVOID EntryContext) {
	DbgMsg("Entered custom QueryRoutine\n");
	RTL_QUERY_REGISTRY_TABLE* ctx = (RTL_QUERY_REGISTRY_TABLE*)Context;
	memcpy(ctx->EntryContext, ValueData, 8);

	return STATUS_SUCCESS;
};

/// <summary>
/// It expects the function offset to be set inside the RURIOffset key at \\Registry\\Machine\\SOFTWARE\\UniHide by the loader
/// </summary>
/// <returns>RaidUnitRegisterInterfaces function offset</returns>
UINT64 GetRURI() {
	UINT64 offset = 0;
	
	OBJECT_ATTRIBUTES KeyAttributes;
	UNICODE_STRING Name;
	UNICODE_STRING ValueName;
	HANDLE hKey;
	RtlInitUnicodeString(&Name, globals::UniHideKeysPath);
	RtlInitUnicodeString(&ValueName, L"RURIOffset");

	InitializeObjectAttributes(&KeyAttributes,
		&Name,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	ULONG keyFlag = REG_OPENED_EXISTING_KEY;
	NTSTATUS status = ZwCreateKey(&hKey, READ_CONTROL, &KeyAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, &keyFlag);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not open registry key: %x", status);
		return offset;
	}

	ULONG size = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(UINT64);
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)kMalloc(size);

	ULONG out = 0;

	status = ZwQueryValueKey(hKey, &ValueName, KeyValuePartialInformation, pInfo, size, &out);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not get registry key value: %x:%x", status, out);
	}
	else {
		offset = *(UINT64*)&pInfo->Data[0];
		DbgMsg("Found registry value: %llx", offset);
	}

	kDelete(pInfo);
	ZwClose(hKey);

	return offset;
}

UINT64 GetDEDFP() {
	UINT64 offset = 0;

	OBJECT_ATTRIBUTES KeyAttributes;
	UNICODE_STRING Name;
	UNICODE_STRING ValueName;
	HANDLE hKey;
	RtlInitUnicodeString(&Name, globals::UniHideKeysPath);
	RtlInitUnicodeString(&ValueName, L"DEDFPOffset");

	InitializeObjectAttributes(&KeyAttributes,
		&Name,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	ULONG keyFlag = REG_OPENED_EXISTING_KEY;
	NTSTATUS status = ZwCreateKey(&hKey, READ_CONTROL, &KeyAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, &keyFlag);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not open registry key: %x", status);
		return offset;
	}

	ULONG size = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(UINT64);
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)kMalloc(size);

	ULONG out = 0;

	status = ZwQueryValueKey(hKey, &ValueName, KeyValuePartialInformation, pInfo, size, &out);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not get registry key value: %x:%x", status, out);
	}
	else {
		offset = *(UINT64*)&pInfo->Data[0];
		DbgMsg("Found registry value: %llx", offset);
	}

	kDelete(pInfo);
	ZwClose(hKey);

	return offset;
}

void SpoofHDD1()
{
	DbgMsg("Attempting to spoof HDD: 1");

	INT HDD_count = 0;
	CHAR HDDSPOOF_BUFFER[MAX_HDDS][32] = { 0x20 };
	CHAR HDDORG_BUFFER[MAX_HDDS][32] = { 0 };

	if (!pRegDevInt) {
		DbgMsg("Finding RaidUnitRegisterInterface...");
		PVOID address = GetKernelAddress("storport.sys");
		UINT64 RegDevIntOFF = GetRURI();

		if (!RegDevIntOFF) {
			DbgMsg("Function offset not found");
			return;
		}

		pRegDevInt = (RaidUnitRegisterInterfaces)((UINT64)address + RegDevIntOFF);
	}

	PDEVICE_OBJECT pObject = NULL;
	PFILE_OBJECT pFileObj = NULL;

	UNICODE_STRING DestinationString;
	RtlInitUnicodeString(&DestinationString, L"\\Device\\RaidPort0");

	NTSTATUS status = IoGetDeviceObjectPointer(&DestinationString, FILE_READ_DATA, &pFileObj, &pObject);

	PDRIVER_OBJECT pDriver = pObject->DriverObject;

	PDEVICE_OBJECT pDevice = pDriver->DeviceObject;

	while (pDevice->NextDevice != NULL)
	{
		if (pDevice->DeviceType == FILE_DEVICE_DISK)
		{
			PHDD_EXTENSION pDeviceHDD = (PHDD_EXTENSION)pDevice->DeviceExtension;

			CHAR HDDSPOOFED_TMP[32] = { 0x0 };
			RandString<CHAR>((CHAR*)&HDDSPOOFED_TMP, SERIAL_MAX_LENGTH - 1);

			//Can be optimised...
			for (int i = 1; i <= SERIAL_MAX_LENGTH + 1; i = i + 2)
			{
				memcpy(&HDDORG_BUFFER[HDD_count][i - 1], &pDeviceHDD->pHDDSerial[i], sizeof(CHAR));
				memcpy(&HDDORG_BUFFER[HDD_count][i], &pDeviceHDD->pHDDSerial[i - 1], sizeof(CHAR));

				memcpy(&HDDSPOOF_BUFFER[HDD_count][i - 1], &HDDSPOOFED_TMP[i], sizeof(CHAR));
				memcpy(&HDDSPOOF_BUFFER[HDD_count][i], &HDDSPOOFED_TMP[i - 1], sizeof(CHAR));
			}

			strcpy(pDeviceHDD->pHDDSerial, (char*) & HDDSPOOFED_TMP);

			//reset the registry entries to the faked serials
			pRegDevInt(pDeviceHDD);

			HDD_count++;
		}

		pDevice = pDevice->NextDevice;
	}

	if(status == STATUS_SUCCESS)
		DbgMsg("Spoofed HDD successfully: 1");
}

void SpoofHDD2() {
	DbgMsg("Attempting to spoof HDD: 2");

	if (!pDiskFailPrediction) {
		DbgMsg("Finding RaidUnitRegisterInterface...");
		PVOID address = GetKernelAddress("disk.sys");
		UINT64 RegDevIntOFF = GetDEDFP();

		if (!RegDevIntOFF) {
			DbgMsg("Function offset not found");
			return;
		}

		pDiskFailPrediction = (DISK_FAIL_PREDICTION)((UINT64)address + RegDevIntOFF);
	}

	if (pDiskFailPrediction) {
		DbgMsg("Found DiskEnableDisableFailurePrediction at: %p", pDiskFailPrediction);
	}
	else {
		DbgMsg("Could not find DiskEnableDisableFailurePrediction");
		return;
	}



	DbgMsg("Spoofed HDD successfully: 2");
}