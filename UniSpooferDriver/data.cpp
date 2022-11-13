#pragma once
#include "data.h"
#include "cpp.h"
#include "macros.h"

wchar_t Globals::RURIKeyPath[512] = {0};

void Globals::Init()
{
    wcscpy(RURIKeyPath, L"\\Registry\\Machine\\SOFTWARE\\UniHide");
}
