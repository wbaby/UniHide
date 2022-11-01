#include "utils.h"

UNICODE_STRING GetModuleNameFromPath(PUNICODE_STRING path)
{
	UNICODE_STRING s;
	
	char* char_arr = (char*)kMalloc((size_t)path->Length + 1, NonPagedPool, false);
	wcstombs(char_arr, path->Buffer, (size_t)path->Length);
	string* full_path = string::create(char_arr);

	int index = full_path->last_of('\\');
	string* modName = full_path->substring(index + 1);
	PCWSTR wstr = (PCWSTR)kMalloc(((size_t)modName->len + 1) * 2);
	mbstowcs((wchar_t*)wstr, (char*)modName->pBuffer, modName->len);
	RtlInitUnicodeString(&s, wstr);

	full_path->Dispose();
	modName->Dispose();
	kDelete(full_path);
	kDelete(modName);
	kDelete(char_arr, false);

	return s;
}
