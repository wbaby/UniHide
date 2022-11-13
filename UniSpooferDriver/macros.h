#pragma once

#define _DEBUG
#define _KDMAPPED

#ifdef _DEBUG
#define DbgMsg(x, ...) DbgPrintEx(0, 0, x, __VA_ARGS__)
#else 
#define DbgMsg(x, ...)
#pragma warning (disable:4390)
#endif

#ifndef _KDMAPPED
#define EntryInit DriverEntryInit
#define EntryPoint DriverEntry
#else
#define EntryInit DriverEntry
#define EntryPoint DriverEntryInit
#endif

//Functions

//Make sure to include cpp.h to use this macro
#define ZeroMemory(dst, len) { memset(dst, 0, len); }

//Constants
#define DRIVER_TAG 'mDcA'
#define DRIVER_NAME "Loader"
#define DRIVER_LNK_NAME "Loader"
#define DRIVER_IMPORT_API extern "C"
