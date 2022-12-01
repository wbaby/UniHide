#include "StringEx.h"

#pragma warning (disable : 4267)

//Length without null terminator
void mbstowcs(wchar_t* dst, char* src, size_t len)
{
	for (size_t i = 0; i < len + 1; i++) {
		*(char*)&dst[i] = src[i];
		*((char*)&dst[i] + 1) = 0x00;
	}
}

void wcstombs(char* dst, wchar_t* src, size_t len) {
	for (size_t i = 0; i < len + 1; i++) {
		dst[i] = *(char*)&src[i];
	}
}

string::string()
{
	len = 0;
	pBuffer = nullptr;
}

string::string(const char* pString) {
	len = strlen(pString);
	pBuffer = (char*)cpp::kMalloc((size_t)len + 1, 'tSyM');
	strcpy((char*)pBuffer, pString);
}

string::string(string& obj)
{
	len = obj.len;
	pBuffer = obj.pBuffer;
}

void string::Dispose()
{
	if (pBuffer != nullptr) {
		//DbgMsg("Disposing of string %s \n", pBuffer);
		cpp::kFree((void*)pBuffer);
		pBuffer = nullptr;
	}
}

string* string::create(const char* pString)
{
	auto pNewString = (string*)cpp::kMalloc((size_t)sizeof(string), 'tSyM');
	pNewString->len = strlen(pString);
	pNewString->pBuffer = (char*)cpp::kMalloc((size_t)pNewString->len + 1);
	strcpy((char*)pNewString->pBuffer, pString);
	return pNewString;
}

string::~string()
{
	Dispose();
}

UNICODE_STRING string::unicode()
{
	UNICODE_STRING retValue;
	wchar_t buf[255];
	mbstowcs(buf, (char*)pBuffer, (size_t)len + 1);
	RtlInitUnicodeString(&retValue, buf);
	return retValue;
}

string* string::substring(int index)
{
	char* char_arr = (char*)cpp::kMalloc((size_t)len - index + 1);
	memcpy(char_arr, pBuffer + index, (size_t)len - index);
	char_arr[len - index] = 0;
	string* pRetValue = string::create(char_arr);

	cpp::kFree(char_arr);
	return pRetValue;
}

string* string::substring(int index, int length)
{
	char* char_arr = (char*)cpp::kMalloc((size_t)len - index + 1);
	if (len > index + length) {
		memcpy(char_arr + index, pBuffer, (size_t)len - index);
		char_arr[len - index + 1] = 0;
	}
	else {
		memcpy(char_arr, pBuffer, length);
		char_arr[length + 1] = 0;
	}
	string* pRetValue = string::create(char_arr);
	cpp::kFree(char_arr);
	return pRetValue;
}

int string::last_of(char to_find)
{
	for (int i = len; i > 0; i--) {
		if (pBuffer[i] == to_find)
			return i;
	}
	return 0;
}

const char* string::c_str() {
	return pBuffer;
}