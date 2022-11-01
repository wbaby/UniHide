#pragma once
#include "common.h"

#pragma warning (disable: 6001)

struct DEVICE_ID_DESCRIPTOR_HOLDER {
	DWORD NumberOfIdentifiers = 0;
	std::vector<BYTE> Identifiers;
	char drive_letter;

	void Init(PSTORAGE_DEVICE_ID_DESCRIPTOR pSdid, char driveLetter);
};

struct DEVICE_DESCRIPTOR_HOLDER {
	std::string serial;
	char driver_letter;

	void Init(PSTORAGE_DEVICE_DESCRIPTOR pSdd, char driveLetter);
};

class DriveDetector
{
private:
	DWORD logical_drives;

	size_t getDeviceIDs();
	size_t getDeviceSerials();
	size_t getRcvDrivesData();

public:
	std::vector<DEVICE_ID_DESCRIPTOR_HOLDER> id_descriptors;
	std::vector<DEVICE_DESCRIPTOR_HOLDER> descriptors;

	DriveDetector();
};

