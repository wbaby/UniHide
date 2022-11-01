#pragma once

#include "VectorEx.h"

//Implements shit for disposing of unused memory
struct Collector {
private:
	static vector<void*>* myGarbage;
public:
	static void Init();
	static void Add(void* p);
	static void Remove(void* p);
	static void Clean();
};
