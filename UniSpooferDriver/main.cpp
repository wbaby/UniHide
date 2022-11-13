#include "main.h"

NTSTATUS EntryPoint(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    DbgMsg("Redirecting to Driver Entry..");
    UNICODE_STRING driver_name;
    NTSTATUS status;
    RtlInitUnicodeString(&driver_name, L"\\Driver\\Loader");
    status = IoCreateDriver(&driver_name, &EntryInit);

    return status;
}

NTSTATUS EntryInit(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    pDriverObject->DriverUnload = (PDRIVER_UNLOAD)UnloadDriver;

    Collector::Init();
    globals::Init();

    DbgMsg("Successfully initialized driver");

    SpoofHDD();

    return STATUS_SUCCESS;
}

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);

    DbgMsg("Driver unloading...\n");

    Collector::Clean();

    DbgMsg("Driver unloaded successfully\n");

    return STATUS_SUCCESS;
}