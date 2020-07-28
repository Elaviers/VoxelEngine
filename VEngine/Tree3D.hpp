#pragma once
#include <ELMaths/Vector3.hpp>

//3 dimensional KD tree
//todo

template <typename T, typename V>
class Tree3D
{
	class Node
	{
	public:
		Vector3T<T> coords;
		Node* left;
		Node* right;

		V value;

		Node() {}
		~Node() {}

		void DeleteChildren()
		{
			if (left)
			{
				left->DeleteChildren();
				delete left;
			}
		}
	};

	Node* _data;

public:
	Tree3D() {}
	~Tree3D() {}
};

