#include "pdbparse.h"

DWORD g_dwMachineType = CV_CFL_80386;

PdbParser::PdbParser(std::wstring path)
{
    DWORD dwMachType = 0;
    HRESULT hr = CoInitialize(NULL);

    // Obtain access to the provider

    hr = CoCreateInstance(__uuidof(DiaSource),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IDiaDataSource),
        (void**)&pDiaDataSource);

    if (FAILED(hr)) {
        wprintf(L"CoCreateInstance failed - HRESULT = %08X\n", hr);
        printf("Trying to register COM object...\n");
        system("regsvr32 /s msdia140.dll");
        printf("Try to restart the application\n");
        return;
    }

    int i = path.find_last_of('/\\');
    std::wstring fileName = path.substr(i + 1);

    hr = pDiaDataSource->loadDataFromPdb(fileName.c_str());

    if (FAILED(hr)) {
        wprintf(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);

        return;
    }

    hr = pDiaDataSource->openSession(&pDiaSession);

    if (FAILED(hr)) {
        wprintf(L"openSession failed - HRESULT = %08X\n", hr);

        return;
    }

    hr = pDiaSession->get_globalScope(&pGlobalSymbol);

    if (hr != S_OK) {
        wprintf(L"get_globalScope failed\n");

        return;
    }

    if (pGlobalSymbol->get_machineType(&dwMachType) == S_OK) {
        switch (dwMachType) {
        case IMAGE_FILE_MACHINE_I386: g_dwMachineType = CV_CFL_80386; break;
        case IMAGE_FILE_MACHINE_IA64: g_dwMachineType = CV_CFL_IA64; break;
        case IMAGE_FILE_MACHINE_AMD64: g_dwMachineType = CV_CFL_AMD64; break;
        }
    }
}

PdbParser::~PdbParser()
{
    if (pGlobalSymbol) {
        pGlobalSymbol->Release();
        pGlobalSymbol = NULL;
    }

    if (pDiaSession) {
        pDiaSession->Release();
        pDiaSession = NULL;
    }

    CoUninitialize();
}

size_t PdbParser::GetFunctionRVA(std::wstring funcName)
{
    size_t rva = 0;

    IDiaEnumSymbols* pEnumSymbols;

    if (FAILED(pGlobalSymbol->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))) {
        return false;
    }

    IDiaSymbol* pSymbol;
    ULONG celt = 0;

    int i = 0;
    while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
        BSTR bstrUndname;

        if (pSymbol->get_undecoratedNameEx(0x1000, &bstrUndname) == S_OK
            && !wcscmp(funcName.c_str(), bstrUndname)) {

            pSymbol->get_relativeVirtualAddress((DWORD*)&rva);

            break;
        }


        pSymbol->Release();
        i++;
    }

    pEnumSymbols->Release();

    return rva;
}
