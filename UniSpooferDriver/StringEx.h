#pragma once
#include "cpp.h"
#include <ntifs.h>

void mbstowcs(wchar_t* dst, char* src, size_t len);
void wcstombs(char* dst, wchar_t* src, size_t len);

//This class supposes it is being initialized from a cstring const char* (for now at least)
struct string {
public:
	int len;
	const char* pBuffer;

	string();
	//Constructors-Desctructors
	string(const char* pString);
	string(string& obj);
	static string* create(const char* pString);
	~string();
	void Dispose(); //c# style memory free
	//Other functions 
	const char* c_str();
	UNICODE_STRING unicode();
	string* substring(int index);
	string* substring(int index, int length);
	int last_of(char to_find);
	//Operators
	string& operator+(const char* LpCStr) {
		if (LpCStr == nullptr)
			return *this;

		string* ret = (string*)cpp::kMalloc(sizeof(string), 'tSyM');
		auto length = len + (int)strlen(LpCStr);
		auto buffer = (char*)cpp::kMalloc((size_t)length + 1);
		if (buffer == nullptr) return *this;
		ZeroMemory((void*)buffer, (size_t)length + 1);
		memcpy((void*)buffer, pBuffer, len);
		strcat((char*)buffer, LpCStr);
		ret->pBuffer = buffer;
		ret->len = length;

		return *ret;
	};
	void operator=(const string& obj) {
		if (&obj == this)
			return;
		Dispose();
		pBuffer = (char*)cpp::kMalloc((size_t)obj.len + 1);
		ZeroMemory((void*)pBuffer, (size_t)obj.len + 1);
		memcpy((void*)pBuffer, obj.pBuffer, obj.len);
		len = obj.len;
	};
};
