#include "collector.h"

vector<MemoryAddress>* Collector::myGarbage = nullptr;

void Collector::Init()
{
	if (myGarbage)
		return;
	myGarbage = vector<MemoryAddress>::create();
	DbgMsg("Initialized collector");
}

void Collector::Add(void* p, cpp::MemoryType type, size_t sz)
{
	MemoryAddress mAddy = {
		p,
		type,
		sz
	};

	myGarbage->Append(mAddy, false);
}

void Collector::Remove(void* p)
{
	auto fnDeleteWhere = [p](MemoryAddress mObj) {
		return mObj.pMemory == p;
	};

	myGarbage->RemoveWhere(fnDeleteWhere, false);
}

void Collector::Clean()
{
	DbgMsg("Number of leaked memory pools: %d", myGarbage->Length());
	//Dispose of pointers stored inside node's objects
	node<MemoryAddress>* curNode = myGarbage->First();
	for (int i = 0; i < myGarbage->Length(); i++) {
		auto nextNode = curNode->fLink;
		cpp::kFree(curNode->obj.pMemory, false);
		cpp::kFree(curNode, false);
		curNode = nextNode;
	}
	DbgMsg("Disposed of leaked memory");
	//Dispose of nodes and vector object
	cpp::kFree((void*)myGarbage, false);
	DbgMsg("Disposed of garbage collector");
}

void FreeMemory(MemoryAddress& memAddr) {
	switch (memAddr.type) {
	case cpp::NonPaged:
		ExFreePool(memAddr.pMemory);
		break;
	case cpp::NonCached:
		MmFreeNonCachedMemory(memAddr.pMemory, memAddr.szMemory);
		break;
	case cpp::Continuous:
		MmFreeContiguousMemory(memAddr.pMemory);
		break;
	}
}

void Collector::Clean(void* pMemory, bool collect)
{
	//Dispose of pointers stored inside node's objects
	bool bFound = false;
	node<MemoryAddress>* curNode = myGarbage->First();
	for (int i = 0; i < myGarbage->Length(); i++) {
		if (curNode->obj.pMemory == pMemory) {
			bFound = true;
			break;
		}
		curNode = curNode->fLink;
	}
	if (!bFound) {
		ExFreePool(pMemory);
		return;
	}
	
	//Dispose of nodes and vector object
	FreeMemory(curNode->obj);

	if (collect) {
		// Remove from linked list
		auto fnWhere = [pMemory](MemoryAddress& memAddr) {
			return pMemory == memAddr.pMemory;
		};

		myGarbage->RemoveWhere(fnWhere, false);
	}
}
