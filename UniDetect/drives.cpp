#include "drives.h"

DriveDetector::DriveDetector()
{
	this->logical_drives = GetLogicalDrives();
	this->getDeviceIDs();
	this->getDeviceSerials();
	this->getSmart();
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

size_t DriveDetector::getSmart(void) {
	int done = FALSE;
	int drive = 0;

	for (drive = 0; drive < MAX_IDE_DRIVES; drive++) {
		HANDLE hPhysicalDriveIOCTL = 0;

		//  Try to get a handle to PhysicalDrive IOCTL, report failure
		//  and exit if can't.
		char driveName[256] = {0};

		sprintf_s(driveName, "\\\\.\\PhysicalDrive%d", drive);

		//  Windows NT, Windows 2000, must have admin rights
		hPhysicalDriveIOCTL = CreateFileA(driveName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);

		if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
		{
			printf("\nReadPhysicalDriveInNTWithAdminRights ERROR"
				"\nCreateFile(%s) returned INVALID_HANDLE_VALUE: %x\n",
				driveName, GetLastError());
			return 0;
		}

		GETVERSIONOUTPARAMS VersionParams;
		DWORD cbBytesReturned = 0;

		// Get the version, etc of PhysicalDrive IOCTL
		memset((void*)&VersionParams, 0, sizeof(VersionParams));

		if (!DeviceIoControl(hPhysicalDriveIOCTL, DFP_GET_VERSION,
			NULL,
			0,
			&VersionParams,
			sizeof(VersionParams),
			&cbBytesReturned, NULL))
		{
			printf("\n%d ReadPhysicalDriveInNTWithAdminRights ERROR"
				"\nDeviceIoControl(%d, DFP_GET_VERSION) returned 0, error is %d\n",
				__LINE__, (int)hPhysicalDriveIOCTL, (int)GetLastError());
		}

		// If there is a IDE device at number "i" issue commands
		// to the device
		if (VersionParams.bIDEDeviceMap <= 0)
		{
			printf("\n%d ReadPhysicalDriveInNTWithAdminRights ERROR"
				"\nNo device found at position %d (%d)\n",
				__LINE__, (int)drive, (int)VersionParams.bIDEDeviceMap);
			return 0;
		}

		BYTE bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
		SENDCMDINPARAMS scip;

		// Now, get the ID sector for all IDE devices in the system.
		// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
		// otherwise use the IDE_ATA_IDENTIFY command
		bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? \
			IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

		memset(&scip, 0, sizeof(scip));
		memset(IdOutCmd, 0, sizeof(IdOutCmd));

		if (DoIDENTIFY(hPhysicalDriveIOCTL,
			&scip,
			(PSENDCMDOUTPARAMS)&IdOutCmd,
			(BYTE)bIDCmd,
			(BYTE)drive,
			&cbBytesReturned))
		{
			DWORD diskdata[256];
			int ijk = 0;
			USHORT* pIdSector = (USHORT*)
				((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

			for (ijk = 0; ijk < 256; ijk++)
				diskdata[ijk] = pIdSector[ijk];

			PrintIdeInfo(drive, diskdata);

			done = TRUE;
		}

		CloseHandle(hPhysicalDriveIOCTL);
	}
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

// DoIDENTIFY
// FUNCTION: Send an IDENTIFY command to the drive
// bDriveNum = 0-3
// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
BOOL DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
	PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
	PDWORD lpcbBytesReturned)
{
	// Set up data structures for IDENTIFY command.
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;
	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	//pSCIP -> irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;

	// Compute the drive number.
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

	// The command can either be IDE identify or ATAPI identify.
	pSCIP->irDriveRegs.bCommandReg = bIDCmd;
	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	return (DeviceIoControl(hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
		(LPVOID)pSCIP,
		sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP,
		sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
		lpcbBytesReturned, NULL));
}

char* ConvertToString(DWORD diskdata[256], int firstIndex, int lastIndex, char* buf)
{
	int index = 0;
	int position = 0;

	//  each integer has two characters stored in it backwards
	for (index = firstIndex; index <= lastIndex; index++)
	{
		//  get high byte for 1st character
		buf[position++] = (char)(diskdata[index] / 256);

		//  get low byte for 2nd character
		buf[position++] = (char)(diskdata[index] % 256);
	}

	//  end the string 
	buf[position] = '\0';

	//  cut off the trailing blanks
	for (index = position - 1; index > 0 && isspace(buf[index]); index--)
		buf[index] = '\0';

	return buf;
}

void PrintIdeInfo(int drive, DWORD diskdata[256])
{
	char serialNumber[1024];
	char modelNumber[1024];
	char revisionNumber[1024];
	char bufferSize[32];

	__int64 sectors = 0;
	__int64 bytes = 0;

	//  copy the hard drive serial number to the buffer
	ConvertToString(diskdata, 10, 19, serialNumber);
	ConvertToString(diskdata, 27, 46, modelNumber);
	ConvertToString(diskdata, 23, 26, revisionNumber);
	sprintf_s(bufferSize, "%u", diskdata[21] * 512);

	//if (0 == HardDriveSerialNumber[0] &&
	//	//  serial number must be alphanumeric
	//	//  (but there can be leading spaces on IBM drives)
	//	(isalnum(serialNumber[0]) || isalnum(serialNumber[19])))
	//{
	//	strcpy(HardDriveSerialNumber, serialNumber);
	//	strcpy(HardDriveModelNumber, modelNumber);
	//}

	printf("\nDrive %d - ", drive);

	switch (drive / 2)
	{
	case 0: printf("Primary Controller - ");
		break;
	case 1: printf("Secondary Controller - ");
		break;
	case 2: printf("Tertiary Controller - ");
		break;
	case 3: printf("Quaternary Controller - ");
		break;
	}

	switch (drive % 2)
	{
	case 0: printf("Master drive\n\n");
		break;
	case 1: printf("Slave drive\n\n");
		break;
	}

	printf("Drive Model Number________________: [%s]\n",
		modelNumber);
	printf("Drive Serial Number_______________: [%s]\n",
		serialNumber);
	printf("Drive Controller Revision Number__: [%s]\n",
		revisionNumber);

	printf("Controller Buffer Size on Drive___: %s bytes\n",
		bufferSize);

	printf("Drive Type________________________: ");
	if (diskdata[0] & 0x0080)
		printf("Removable\n");
	else if (diskdata[0] & 0x0040)
		printf("Fixed\n");
	else printf("Unknown\n");

	//  calculate size based on 28 bit or 48 bit addressing
	//  48 bit addressing is reflected by bit 10 of word 83
	if (diskdata[83] & 0x400)
		sectors = diskdata[103] * 65536I64 * 65536I64 * 65536I64 +
		diskdata[102] * 65536I64 * 65536I64 +
		diskdata[101] * 65536I64 +
		diskdata[100];
	else
		sectors = diskdata[61] * 65536 + diskdata[60];
	//  there are 512 bytes in a sector
	bytes = sectors * 512;
	printf("Drive Size________________________: %I64d bytes\n",
		bytes);

	char string1[1000];
	sprintf_s(string1, "Drive%dModelNumber", drive);

	sprintf_s(string1, "Drive%dSerialNumber", drive);

	sprintf_s(string1, "Drive%dControllerRevisionNumber", drive);

	sprintf_s(string1, "Drive%dControllerBufferSize", drive);

	sprintf_s(string1, "Drive%dType", drive);
}