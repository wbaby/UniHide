#pragma once

#define VERSION 0.0.5

#define _DEBUG

#ifndef _CLIENT
#ifdef _DEBUG
#define DbgMsg(x, ...) DbgPrintEx(0, 0, x, __VA_ARGS__)

//If DRIVER_TAG is defined then the new overload will allocate pools of tagged memory
//It needs to be written in reverse since debuggers display such tag in little endian, rDyM becomes MyDr(MyDriver)
#else 
#define DbgMsg(x, ...)
#endif
#endif

//Functions

//Make sure to include cpp.h to use this macro
#define ZeroMemory(dst, len) { memset(dst, 0, len); }

//Constants
#define DRIVER_TAG 'mDcA' 
#define DRIVER_NAME "Loader"
#define DRIVER_LNK_NAME "Loader"
#define DRIVER_IMPORT_API extern "C"
#define INJ_DRIVER		"npcap.sys"

//Define
#define SIOCTL_TYPE 40000
#define IOCTL_HELLO\
 CTL_CODE( SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
