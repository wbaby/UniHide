#pragma once

#include "collector.h"
#include "data.h"
#include "macros.h"
#include "VTx.h"
#include "EPT.h"
#include "hdd.h"

#include <wdm.h>

DRIVER_IMPORT_API NTSTATUS IoCreateDriver(
	IN  PUNICODE_STRING DriverName    OPTIONAL,
	IN  PDRIVER_INITIALIZE InitializationFunction
);

NTSTATUS DriverEntryInit(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);