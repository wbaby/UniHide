#pragma once
#include "data.h"
#include "cpp.h"
#include "macros.h"
#include "utils.h"

wchar_t Globals::UniHideKeysPath[512] = {0};
wchar_t Globals::CurrentDriverName[64] = { 0 };
bool Globals::IsInitialized = false;

vector<PVM_STATE> Globals::vGuestStates;
ULONG Globals::ulProcessorMask = 0;

EventLogger Globals::evLogger;

size_t Globals::szEndFlag = 0x53746f7056697274; //StopVirt in ASCII

void Globals::Init(PDRIVER_OBJECT pDevice)
{
    if (IsInitialized)
        return;
    IsInitialized = true;

    wcscpy(UniHideKeysPath, L"\\Registry\\Machine\\SOFTWARE\\UniHide");

    WCHAR SPOOFER_TMP[17] = { 0x0 };
    RandString<WCHAR>((WCHAR*) &SPOOFER_TMP, 8);

    memcpy(Globals::CurrentDriverName, L"\\Driver\\", 8 * 2);
    memcpy(Globals::CurrentDriverName + 8, SPOOFER_TMP, sizeof(SPOOFER_TMP));

    evLogger.pDevice = pDevice;

    DbgMsg("Successfully initialized global variables");
}
