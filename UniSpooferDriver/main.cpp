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
    Globals::Init(pDriverObject);

    DbgMsg("Current driver name: %ls", Globals::CurrentDriverName);
    
    UNICODE_STRING driver_name;
    NTSTATUS status;
    RtlInitUnicodeString(&driver_name, Globals::CurrentDriverName);
    status = IoCreateDriver(&driver_name, &EntryInit);

    return status;
}

NTSTATUS EntryInit(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    ULONG ulProcessors = 1;
    pDriverObject->DriverUnload = (PDRIVER_UNLOAD)UnloadDriver;

    Collector::Init();
    Globals::Init(pDriverObject);

    if (!VTx::Init()) {
        DbgMsg("There was an error during VTx Initialization...");
        goto _end;
    }
    
    for (size_t i = 0; i < ulProcessors; i++) {
        UINT64 pEtp = EPT::InitEptp(i);
    
        if (!pEtp) {
            DbgMsg("[VMX] Failed to allocate region for EPT struct! Aborting initialization...");
            return FALSE;
        }
    
        Globals::Hyperv::vGuestStates[i]->pEpt = pEtp;
    
        memset((PVOID)Globals::Hyperv::vGuestStates[i]->pGuestMem, 0xF4, 100 * PAGE_SIZE);
    
        //
        // Launching VM for Test (in the 0th virtual processor)
        //
        VTx::VmLaunch(0, (PEPTP)Globals::Hyperv::vGuestStates[i]->pEpt);
    }

_end:
    return STATUS_SUCCESS;
}

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
    UNREFERENCED_PARAMETER(pDriverObject);

    DbgMsg("Driver unloading...");

    Collector::Clean();

    DbgMsg("Driver unloaded successfully");

    return STATUS_SUCCESS;
}