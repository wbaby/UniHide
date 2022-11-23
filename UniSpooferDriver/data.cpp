#pragma once
#include "data.h"
#include "cpp.h"
#include "macros.h"
#include "utils.h"

wchar_t Globals::UniHideKeysPath[512] = {0};
wchar_t Globals::CurrentDriverName[64] = { 0 };
bool Globals::IsInitialized = false;

//Hypervisor related
vector<PVM_STATE> Globals::vGuestStates;
ULONG Globals::ulProcessorCount = 0;

void Globals::Init()
{
    if (IsInitialized)
        return;
    IsInitialized = true;

    wcscpy(UniHideKeysPath, L"\\Registry\\Machine\\SOFTWARE\\UniHide");

    WCHAR SPOOFER_TMP[17] = { 0x0 };
    RandString<WCHAR>((WCHAR*) &SPOOFER_TMP, 8);

    memcpy(globals::CurrentDriverName, L"\\Driver\\", 8 * 2);
    memcpy(globals::CurrentDriverName + 8, SPOOFER_TMP, sizeof(SPOOFER_TMP));

    DbgMsg("Successfully initialized global variables");
}
