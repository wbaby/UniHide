#include "hdd.h"

using namespace hdd;

Error hdd::SetupHDD()
{
	auto parser = PdbParser(L"C:\\symbols\\storport.pdb\\D7F839E51298E4B81B36CA86B63793571\\storport.pdb");

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
