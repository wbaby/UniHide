#pragma once
#include <combaseapi.h>
#include <string>
#include <iostream>
#include "dia2.h"

class PdbParser {
private:
	IDiaDataSource* pDiaDataSource;
	IDiaSession* pDiaSession;
	IDiaSymbol* pGlobalSymbol;

public:
	PdbParser(std::wstring path);
	~PdbParser();

	size_t GetFunctionRVA(std::wstring funcName);
};
