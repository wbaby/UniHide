#include "drives.h"

DriveDetector::DriveDetector()
{
	this->logical_drives = GetLogicalDrives();
	this->getDeviceIDs();
	this->getDeviceSerials();
	this->getRcvDrivesData();
}

size_t DriveDetector::getDeviceIDs()
{
	char device_name[device_base_string_len] = { 0 };
	strcpy_s(device_name, device_base_string);

	for (int i = 0; i < 25; i++) {
		if (this->logical_drives & (1 << i)) {
			HANDLE handle = INVALID_HANDLE_VALUE;
			char logical_letter = 0x41 + i;
			device_name[device_base_string_index] = logical_letter;

			DBG_PRINTF("Getting id for drive: %c\n", logical_letter);

			handle = CreateFileA(device_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
			if (handle == INVALID_HANDLE_VALUE) {
				DBG_PRINTF("Invalid handle returned\n");
				continue;
			}

			DEVICE_ID_DESCRIPTOR_HOLDER holder;
			PSTORAGE_DEVICE_ID_DESCRIPTOR pSdid = (PSTORAGE_DEVICE_ID_DESCRIPTOR)malloc(sizeof(*pSdid));
			STORAGE_DEVICE_ID_DESCRIPTOR sdid;
			STORAGE_PROPERTY_QUERY spq = { StorageDeviceUniqueIdProperty, PropertyStandardQuery };

			DWORD out_bytes = 0;
			BOOL success = DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), &sdid, sizeof(STORAGE_DEVICE_ID_DESCRIPTOR), &out_bytes, NULL);
			if (GetLastError() == ERROR_MORE_DATA) {
				auto tmp = (PSTORAGE_DEVICE_ID_DESCRIPTOR)malloc(pSdid->Size);
				free(pSdid);
				pSdid = tmp;
				success = DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), pSdid, pSdid->Size, &out_bytes, NULL);
			}
			if (!success) {
				DBG_PRINTF("DeviceIoControl failed :(\n");
				goto end;
			}

			holder.Init(pSdid, logical_letter);
			this->id_descriptors.push_back(holder);
		end:
			PRINT_LAST_ERROR();
			CloseHandle(handle);
			delete pSdid;
		}
	}
	return this->id_descriptors.size();
}

size_t DriveDetector::getDeviceSerials()
{
	char device_name[device_base_string_len] = { 0 };
	strcpy_s(device_name, device_base_string);

	for (int i = 0; i < 25; i++) {
		if (this->logical_drives & (1 << i)) {
			HANDLE handle = INVALID_HANDLE_VALUE;
			char logical_letter = 0x41 + i;
			device_name[device_base_string_index] = logical_letter;

			DBG_PRINTF("Getting serial for drive: %c\n", logical_letter);

			handle = CreateFileA(device_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
			if (handle == INVALID_HANDLE_VALUE) {
				DBG_PRINTF("Invalid handle returned: 0x%x\n", GetLastError());
				continue;
			}

			ULONG size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 0x100;
			PSTORAGE_DEVICE_DESCRIPTOR pSdd = (PSTORAGE_DEVICE_DESCRIPTOR)malloc(size);
			STORAGE_PROPERTY_QUERY spq = { StorageDeviceProperty, PropertyStandardQuery };

			NTSTATUS status;

			do {
				status = STATUS_INSUFFICIENT_RESOURCES;

				
			} while (status == STATUS_BUFFER_OVERFLOW);

			DWORD out_bytes = 0;
			BOOL success = DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), pSdd, size, &out_bytes, NULL);
			if (!success) {
				DBG_PRINTF("DeviceIoControl failed: 0x%x\n", GetLastError());
				PRINT_LAST_ERROR();
				CloseHandle(handle);
				continue;
			}

			DEVICE_DESCRIPTOR_HOLDER holder;
			holder.Init(pSdd, logical_letter);

			this->descriptors.push_back(holder);

		end:
			CloseHandle(handle);
		}
	}

	return this->descriptors.size();
}

size_t DriveDetector::getRcvDrivesData()
{

	return 0;
}

void DEVICE_ID_DESCRIPTOR_HOLDER::Init(PSTORAGE_DEVICE_ID_DESCRIPTOR pSdid, char driveLetter)
{
	this->NumberOfIdentifiers = pSdid->NumberOfIdentifiers;

	for (DWORD i = 0; i < this->NumberOfIdentifiers; i++) {
		this->Identifiers.push_back(pSdid->Identifiers[i]);
	}

	this->drive_letter = driveLetter;
}

void DEVICE_DESCRIPTOR_HOLDER::Init(PSTORAGE_DEVICE_DESCRIPTOR pSdd, char driveLetter)
{
	this->serial = std::string(((char*)pSdd + pSdd->SerialNumberOffset));
	this->driver_letter = driveLetter;
}
