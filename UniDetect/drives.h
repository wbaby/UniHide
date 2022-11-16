#pragma once
#include "common.h"

#pragma warning (disable: 6001)

#define  DFP_GET_VERSION          0x00074080
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

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

typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;      // Binary driver version.
	BYTE bRevision;     // Binary driver revision.
	BYTE bReserved;     // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, * PGETVERSIONOUTPARAMS, * LPGETVERSIONOUTPARAMS;

class DriveDetector
{
private:
	DWORD logical_drives;
	BYTE IdOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];

	size_t getDeviceIDs();
	size_t getDeviceSerials();
	size_t getSmart();

public:
	std::vector<DEVICE_ID_DESCRIPTOR_HOLDER> id_descriptors;
	std::vector<DEVICE_DESCRIPTOR_HOLDER> descriptors;

	DriveDetector();
	
};

#define MAX_IDE_DRIVES 16

char* ConvertToString(DWORD diskdata[256], int firstIndex, int lastIndex, char* buf);
void PrintIdeInfo(int drive, DWORD diskdata[256]);
BOOL DoIDENTIFY(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD);