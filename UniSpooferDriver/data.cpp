#pragma once
#include "data.h"
#include "cpp.h"
#include "macros.h"

ULONGLONG globals::ModuleAddress = 0;
PDEVICE_OBJECT globals::pDeviceObject = nullptr;
PUNICODE_STRING globals::dev = nullptr;
PUNICODE_STRING globals::dos = nullptr;

void Globals::Init()
{
    dev = (PUNICODE_STRING)kMalloc(sizeof(UNICODE_STRING));
    dos = (PUNICODE_STRING)kMalloc(sizeof(UNICODE_STRING));
    RtlInitUnicodeString(globals::dev, L"\\Device\\" DRIVER_NAME);
    RtlInitUnicodeString(globals::dos, L"\\DosDevices\\" DRIVER_LNK_NAME);
}
