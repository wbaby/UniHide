#pragma once
#include "data.h"
#include "cpp.h"
#include "macros.h"
#include "utils.h"

wchar_t Globals::UniHideKeysPath[512] = {0};
wchar_t Globals::CurrentDriverName[64] = { 0 };
bool Globals::IsInitialized = false;

void Globals::Init()
{
    if (IsInitialized)
        return;
    IsInitialized = true;

    wcscpy(UniHideKeysPath, L"\\Registry\\Machine\\SOFTWARE\\UniHide");

    WCHAR HDDSPOOFED_TMP[17] = { 0x0 };
    RandString<WCHAR>((WCHAR*) & HDDSPOOFED_TMP, 8);

    memcpy(globals::CurrentDriverName, L"\\Driver\\", 8 * 2);
    memcpy(globals::CurrentDriverName + 8, HDDSPOOFED_TMP, sizeof(HDDSPOOFED_TMP));
}
