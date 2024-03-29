#include "hdd.h"
#include "classspnp.h"

DECLARE_HANDLE(HKEY);
#define HKEY_LOCAL_MACHINE                  (( HKEY ) (ULONG_PTR)((LONG)0x80000002) )

typedef NTSTATUS(__fastcall* RaidUnitRegisterInterfaces)(PHDD_EXTENSION a1);
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
	RtlInitUnicodeString(&Name, Globals::UniHideKeysPath);
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
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)cpp::kMalloc(size);

	ULONG out = 0;

	status = ZwQueryValueKey(hKey, &ValueName, KeyValuePartialInformation, pInfo, size, &out);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not get registry key value: %x:%x", status, out);
	}
	else {
		offset = *(UINT64*)&pInfo->Data[0];
		DbgMsg("Found registry value: %llx", offset);
	}

	cpp::kFree(pInfo);
	ZwClose(hKey);

	return offset;
}

UINT64 GetDEDFP() {
	UINT64 offset = 0;

	OBJECT_ATTRIBUTES KeyAttributes;
	UNICODE_STRING Name;
	UNICODE_STRING ValueName;
	HANDLE hKey;
	RtlInitUnicodeString(&Name, Globals::UniHideKeysPath);
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
	PKEY_VALUE_PARTIAL_INFORMATION pInfo = (PKEY_VALUE_PARTIAL_INFORMATION)cpp::kMalloc(size);

	ULONG out = 0;

	status = ZwQueryValueKey(hKey, &ValueName, KeyValuePartialInformation, pInfo, size, &out);
	if (status != STATUS_SUCCESS) {
		DbgMsg("Could not get registry key value: %x:%x", status, out);
	}
	else {
		offset = *(UINT64*)&pInfo->Data[0];
		DbgMsg("Found registry value: %llx", offset);
	}

	cpp::kFree(pInfo);
	ZwClose(hKey);

	return offset;
}

void SpoofHDD()
{
	DbgMsg("Attempting to spoof HDD");

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
			status = pRegDevInt(pDeviceHDD);

			if(status == STATUS_SUCCESS)
				DbgMsg("Spoofed device %d with value: %s", HDD_count, HDDSPOOFED_TMP);
			else
				DbgMsg("Could not spoof device %d: %x", HDD_count, status);

			HDD_count++;
		}

		pDevice = pDevice->NextDevice;
	}
}

void DisableAndSpoofSMART() {
	DbgMsg("Disabling SMART functionality");

	if (!pDiskFailPrediction) {
		DbgMsg("Finding DiskFailPrediction...");
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

	UNICODE_STRING driver_disk;
	RtlInitUnicodeString(&driver_disk, L"\\Driver\\Disk");
	UNICODE_STRING Unicode;
	RtlInitUnicodeString(&Unicode, L"IoDriverObjectType");
	const POBJECT_TYPE* IoDriverObjectType = (POBJECT_TYPE*)MmGetSystemRoutineAddress(&Unicode);
	if (!IoDriverObjectType) {
		DbgMsg("Failed to get IoDriverObjectType");
		return;
	}
	RtlInitUnicodeString(&Unicode, L"ObReferenceObjectByName");
	const pfnObReferenceObjectByName ObReferenceObjectByName = (pfnObReferenceObjectByName)MmGetSystemRoutineAddress(&Unicode);
	if (!ObReferenceObjectByName) {
		DbgMsg("Failed to get ObReferenceObjectByName");
		return;
	}

	PDRIVER_OBJECT driver_object = nullptr;
	NTSTATUS status = ObReferenceObjectByName(&driver_disk, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<PVOID*>(&driver_object));
	if (!NT_SUCCESS(status)) {
		DbgMsg("Failed to get disk object address");
		return;
	};

	PDEVICE_OBJECT device_object_list[100]{ 0 };
	RtlZeroMemory(device_object_list, sizeof(device_object_list));

	ULONG number_of_device_objects = 0;
	status = IoEnumerateDeviceObjectList(driver_object, device_object_list, sizeof(device_object_list), &number_of_device_objects);
	if (!NT_SUCCESS(status))
	{
		DbgMsg("IoEnumerateDeviceObjectList failed");
		ObDereferenceObject(driver_object);
		return;
	}

	DbgMsg("number of device objects is : %d \n", number_of_device_objects);

	ULONG disabled = 0;

	for (ULONG i = 0; i < number_of_device_objects; ++i)
	{
		PDEVICE_OBJECT device_object = device_object_list[i];
		if (!device_object) {
			DbgMsg("Device object %d is null", i);
			continue;
		}
		PDEVICE_OBJECT disk = IoGetAttachedDeviceReference(device_object);
		if (disk) {
			KEVENT event = { 0 };
			KeInitializeEvent(&event, NotificationEvent, FALSE);

			PIRP irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_UPDATE_PROPERTIES, disk, 0, 0, 0, 0, 0, &event, 0);
			if (irp) {
				if (STATUS_PENDING == IoCallDriver(disk, irp)) {
					KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, 0);
				}
			}
			else {
				DbgMsg("Failed to build IoControlRequest");
			}

			ObDereferenceObject(disk);
		}
		else {
			DbgMsg("No disk found");
		}

		PFUNCTIONAL_DEVICE_EXTENSION ext = (PFUNCTIONAL_DEVICE_EXTENSION)device_object->DeviceExtension;
		if (!ext) {
			DbgMsg("Device extension for disk %d is NULL", i);
			continue;
		}

		strcpy((PCHAR)ext->DeviceDescriptor + ext->DeviceDescriptor->SerialNumberOffset, SERIAL);

		status = pDiskFailPrediction(device_object->DeviceExtension, false);
		if (NT_SUCCESS(status)) {
			DbgMsg("DiskEnableDisableFailurePrediction success");
			disabled++;
		}
		else {
			DbgMsg("DiskEnableDisableFailurePredition failed: %x", status);
		}
		ObDereferenceObject(device_object);
	}
	ObDereferenceObject(driver_object);

	DbgMsg("Disabled SMART functionality for %d devices", disabled);
}