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
    globals::Init(pDriverObject);

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
    //ULONG ulProcessors = 1;
    pDriverObject->DriverUnload = (PDRIVER_UNLOAD)UnloadDriver;

    Collector::Init();
    globals::Init(pDriverObject);

    //if (!VTx::Init()) {
    //    DbgMsg("There was an error during VTx Initialization...");
    //    goto _end;
    //}
    //
    //for (size_t i = 0; i < ulProcessors; i++) {
    //    UINT64 pEtp = EPT::InitEptp(i);
    //
    //    if (!pEtp) {
    //        DbgMsg("[VMX] Failed to allocate region for EPT struct! Aborting initialization...");
    //        return FALSE;
    //    }
    //
    //    globals::vGuestStates[i]->pEpt = pEtp;
    //
    //    memset((PVOID)globals::vGuestStates[i]->pGuestMem, 0xF4, 100 * PAGE_SIZE);
    //
    //    //
    //    // Launching VM for Test (in the 0th virtual processor)
    //    //
    //
    //    VTx::VmLaunch(0, (PEPTP)globals::vGuestStates[i]->pEpt);
    //}

    globals::evLogger.LogEvent(STATUS_FAILED_DRIVER_ENTRY, L"Driver entry successfully executed for UniHide!");

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