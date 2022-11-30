#pragma once

#include <ntddk.h>

#include "macros.h"

class EventLogger
{
public:
	PDRIVER_OBJECT pDevice;
	
	EventLogger() : pDevice(nullptr) {
	};

	void LogEvent(NTSTATUS ntStatus, const WCHAR* pStr);
};
