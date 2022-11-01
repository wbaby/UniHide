#include "collector.h"

vector<void*>* Collector::myGarbage = nullptr;

void Collector::Init()
{
	myGarbage = vector<void*>::create();
	DbgMsg("Initialized collector\n");
}

void Collector::Add(void* p)
{
	myGarbage->emplace_back<void*>(p, false);
	//DbgMsg("Created new object %d \n", (size_t)p);
}

void Collector::Remove(void* p)
{
	myGarbage->Remove(p, false);
	//DbgMsg("Destroyed object %d \n", (size_t)p);
}

void Collector::Clean()
{
	DbgMsg("Number of leaked memory pools: %d \n", myGarbage->Length());
	//Dispose of pointers stored inside node's objects
	node<void*>* curNode = myGarbage->First();
	for (int i = 0; i < myGarbage->Length(); i++) {
		auto nextNode = curNode->fLink;
		kDelete(curNode->obj, false);
		kDelete(curNode, false);
		//DbgMsg("Deleted object at %d \n", (size_t)curNode->obj);
		curNode = nextNode;
	}
	DbgMsg("Disposed of leaked memory\n");
	//Dispose of nodes and vector object
	kDelete((void*)myGarbage, false);
	DbgMsg("Disposed of garbage collector\n");
}
