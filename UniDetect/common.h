#pragma once

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <ntstatus.h>

constexpr char device_base_string[] = "\\\\.\\X:";
constexpr int device_base_string_len = sizeof(device_base_string);
constexpr int device_base_string_index = device_base_string_len - 3;

#ifdef _DEBUG
#define PRINT_LAST_ERROR() printf("Last error: 0x%x\n", GetLastError())
#define DBG_PRINTF(x, ...) printf(x, __VA_ARGS__)
#else
#define PRINT_LAST_ERROR()
#define DBG_PRINTF(x, ...)
#endif