#pragma once

#include "VectorEx.h"

struct MemoryAddress {
	cpp::MemoryType type;
	PVOID pMemory;
	size_t szMemory;	// If 0 it means the size is not important for this type of memory 

	MemoryAddress(PVOID pMemory, cpp::MemoryType type) :
		type(type),
		pMemory(pMemory), 
		szMemory(0) 
	{}

	MemoryAddress(PVOID pMemory, cpp::MemoryType type, size_t szMemory):
		type(type),
		pMemory(pMemory),
		szMemory(szMemory)
	{}
};

//Implements shit for disposing of unused memory
struct Collector {
private:
	static vector<MemoryAddress>* myGarbage;
public:
	static void Init();
	static void Add(void* p, cpp::MemoryType type, size_t sz = 0);
	static void Remove(void* p);
	static void Clean();
	static void Clean(void* pMemory, bool collect = true);
};
