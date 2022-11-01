#pragma once

#include "cpp.h"

template <typename T>
struct node
{
	T obj;
	node<T>* fLink;
	node<T>* bLink;

	static node<T>* create(T* _obj = nullptr, node<T>* _bLink = nullptr, bool collect = true) {
		if (_obj == nullptr) return nullptr;
		node<T>* n = (node<T>*) kMalloc(sizeof(node<T>), NonPagedPool, collect);

		if (n == nullptr) return nullptr;

		n->obj = *_obj;

		n->fLink = n;
		if (_bLink == nullptr)
			n->bLink = n;
		else
			n->bLink = _bLink;
		return n;
	};
};

//Here vector is a single linked list of which last node fLink points to itself
template <class T>
class vector
{
public:
	//Constructors - Destructors
	vector() {
		firstNode = (node<T>*)this;
		lastNode = firstNode;
		length = 0;
	}
	vector(T& obj) {
		firstNode = node<T>::create(&obj);
		lastNode = firstNode;
		length = 1;
	};
	vector(T obj) {
		firstNode = node<T>::create(&obj);
		lastNode = firstNode;
		length = 1;
	};
	static vector<T>* create() {
		auto vec = (vector<void*>*)ExAllocatePoolWithTag(NonPagedPool, sizeof(vector<void*>), 'eVyM');
		new (vec) vector<void*>;
		return vec;
	}
	//This will dispose of every node, but the vec obj must be disposed of manually
	void Dispose(bool collect = true) {
		node<T>* curNode = firstNode;
		for (int i = 0; i < length; i++) {
			auto nextNode = curNode->fLink;
			kDelete((void*)curNode, collect);
			DbgMsg("Deleted node %d \n", (size_t)curNode);
			curNode = nextNode;
		}
	};

	//Methods
	int Length() {
		return length;
	}
	node<T>* First() {
		return firstNode;
	};
	node<T>* Last() {
		return lastNode;
	};
	void Remove(T& obj, bool collect = true) {
		node<T>* curNode = firstNode;
		while (curNode->obj != obj && curNode != lastNode) {
			curNode = curNode->fLink;
		}
		if (curNode == firstNode) {
			firstNode = curNode->fLink;
			firstNode->bLink = firstNode;
		}
		else if (curNode == lastNode) {
			if (curNode->obj == obj) {
				lastNode = curNode->bLink;
				lastNode->fLink = lastNode;
			}
			else {
				return;
			}
		}
		else {
			curNode->bLink->fLink = curNode->fLink;
			curNode->fLink->bLink = curNode->bLink;
		}
		kDelete(curNode, collect);
		length--;
	};
	bool Append(T& obj) {
		node<T>* n = node<T>::create(&obj, lastNode);
		if (lastNode == firstNode)
			lastNode = n;
		n->bLink = lastNode;
		lastNode->fLink = n;
		lastNode = n;
		n->fLink = n;

		if ((size_t)firstNode == (size_t)this)
			firstNode = lastNode;
		if (firstNode->fLink == firstNode) {
			firstNode->fLink = lastNode;
		}
		length++;
		return true;
	};
	template <class ... C>
	bool emplace_back(C ... c, bool collect = true) {
		T obj(c ...);
		node<T>* n = node<T>::create(&obj, lastNode, collect);

		if (lastNode == firstNode)
			lastNode = n;
		n->bLink = lastNode;
		lastNode->fLink = n;
		lastNode = n;
		n->fLink = n;

		if ((size_t)firstNode == (size_t)this)
			firstNode = lastNode;
		if (firstNode->fLink == firstNode) {
			firstNode->fLink = lastNode;
		}
		length++;
		return true;
	}
	T Pop(unsigned int index = length) {
		T ret;
		if (index + 1 == length) {
			lastNode->bLink->fLink = lastNode->bLink;
			lastNode = lastNode->bLink;
			return lastNode->obj;
		}
		node<T>* curNode = nullptr;
		if (index < length / 2) {
			curNode = firstNode;
			int t = 0;
			while (t != index) {
				curNode = curNode->fLink;
				t++;
			}
			if (!t) {
				firstNode = firstNode->fLink;
				firstNode->fLink->bLink = firstNode->fLink;
			}
		}
		else {
			curNode = lastNode;
			int t = length - 1;
			while (t != index) {
				curNode = curNode->bLink;
				t--;
			}

		}
		ret = curNode->obj;
		curNode->bLink->fLink = curNode->fLink;
		curNode->fLink->bLink = curNode->bLink;
		delete(curNode);
		length--;
		return ret;
	};

	//Operators
	const T& operator[](int i) const {
		return BracketOverload(i);
	};
	T& operator[](int i) {
		return BracketOverload(i);
	};
	vector<T>& operator =(vector<T>& rhs) {
		this->firstNode = rhs->firstNode;
		this->lastNode = rhs->lastNode;
		this->length = rhs->length;

		return *this;
	}
private:
	//Private functions used by overloads
	T& BracketOverload(int i) {
		if (i > length)
			return lastNode->obj;

		node<T>* curNode;
		if (i < length / 2) {
			curNode = firstNode;
			int t = 0;
			while (t != i) {
				curNode = curNode->fLink;
				t++;
			}
		}
		else {
			curNode = lastNode;
			int t = length - 1;
			while (t != i) {
				curNode = curNode->bLink;
				t--;
			}

		}

		return curNode->obj;
	};

	//Variables
	node<T>* firstNode;
	node<T>* lastNode;
	int length;
};