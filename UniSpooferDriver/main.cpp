#include "main.h"

void HelloWorld() {
    DbgMsg("Hello from driver land!");
}

void HelloWorldHook() {
    DbgMsg("Hello from hook!");
}

NTSTATUS EntryPoint(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    DbgMsg("Redirecting to Driver Entry..");

    Collector::Init();
    globals::Init();

    DbgMsg("Current driver name: %ls", globals::CurrentDriverName);
    
    UNICODE_STRING driver_name;
    NTSTATUS status;
    RtlInitUnicodeString(&driver_name, globals::CurrentDriverName);
    status = IoCreateDriver(&driver_name, &EntryInit);

    return status;
}

NTSTATUS EntryInit(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    pDriverObject->DriverUnload = (PDRIVER_UNLOAD)UnloadDriver;

    Collector::Init();
    globals::Init();
    VTx::Init();

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