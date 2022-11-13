#include "hdd.h"

using namespace hdd;

Error hdd::SetupHDD()
{
	if (!std::filesystem::exists(".\\storport.pdb")) {
		if (!std::filesystem::exists("PDBDownloader.exe")) {
			printf("PDBDownloader not found, please tr to restart or reinstall the application\n");
			return Error::NotFound;
		}
		system("PDBDownloader.exe C:\\Windows\\System32\\drivers\\storport.sys .\\");
	}

	auto parser = PdbParser(L".\\storport.pdb");

	size_t dwRVA = parser.GetFunctionRVA(L"RaidUnitRegisterInterfaces");
	
	if (!dwRVA) {
		return Error::NotFound;
	}

	DWORD dwDisposition;
	HKEY hkey;
	NTSTATUS status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\UniHide",
		0, NULL, 0,
		KEY_WRITE, NULL,
		&hkey, &dwDisposition);

	if (status == ERROR_SUCCESS) {
		
		status = RegSetValueExA(hkey,
			"RURIOffset",
			0,
			REG_QWORD,
			(BYTE*)&dwRVA,
			8);

		if (status == ERROR_SUCCESS) {

			return Error::Success;
		}
	}
	else {
		return Error::AccessDenied;
	}
}
