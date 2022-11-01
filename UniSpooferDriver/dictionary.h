#pragma once

#include <VectorEx.h>

template <class T, class Y>
struct DictionaryNode {
	T key;
	Y value;

	DictionaryNode(T& k, Y& v) {
		key = k;
		value = v;
	}
};

template <class T, class Y>
class Dictionary {
private:
	vector<DictionaryNode<T, Y>> internalVector;
public:
	Dictionary() {
		//new (&internalVector) vector<DictionaryNode<T, Y>>;
	};
	~Dictionary() {
		internalVector.Dispose();
	};
	void Append(T key, Y value) {
		internalVector.emplace_back(key, value);
	};
	const Y& operator [](const T&& key) const {
		for (size_t i = 0; i < internalVector.Length(); i++) {
			auto pNode = &internalVector[i];
			if (pNode->key == key)
				return pNode->value;
		}
		return internalVector[0];
	}
	Y& operator [](const T&& key) {
		for (size_t i = 0; i < internalVector.Length(); i++) {
			auto pNode = &internalVector[i];
			if (pNode->key == key)
				return pNode->value;
		}
		return internalVector[0].value;
	}
};