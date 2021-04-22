#pragma once
#include <ELCore/Concepts.hpp>

template <typename T, typename COMPONENT_T = int>
class Tree3D
{
private:
	struct Node
	{
		COMPONENT_T location[3];
		T value;

		int axis;
		Node* lesser;
		Node* greater;

		template <typename... Args>
		Node(Args ...constructArgs) : value(static_cast<Args&&...>(constructArgs)...) {}
	};

	Node* _tree;

	template <typename... Args>
	Node* _FindOrCreate(bool& created, const COMPONENT_T location[3], Args&&... constructArgs)
	{
		created = false;
		Node* node;

		if (_tree)
		{
			node = _tree;
			while (node)
			{
				if (location[0] == node->location[0] && location[1] == node->location[1] && location[2] == node->location[2])
					return node;

				Node*& next = location[node->axis] < node->location[node->axis] ? node->lesser : node->greater;
				if (!next)
				{
					next = new Node(static_cast<Args&&...>(constructArgs)...);
					next->axis = node->axis >= 2 ? 0 : (node->axis + 1);
					node = next;
					created = true;
					break;
				}

				node = next;
			}
		}
		else
		{
			_tree = node = new Node;
			node->axis = 0;
		}

		node->location[0] = location[0];
		node->location[1] = location[1];
		node->location[2] = location[2];
		node->lesser = node->greater = nullptr;
		return node;
	}

public:
	Tree3D() : _tree(nullptr) {}

	T& operator[](const COMPONENT_T location[3])
	{
		bool created;
		return _FindOrCreate(created, location)->value;
	}

	T* Get(const COMPONENT_T location[3])
	{
		Node* node = _tree;
		while (node)
		{
			if (location[0] == node->location[0] && location[1] == node->location[1] && location[2] == node->location[2])
				return &node->value;

			node = location[node->axis] < node->location[node->axis] ? node->lesser : node->greater;
		}

		return nullptr;
	}

	const T* Get(const COMPONENT_T location[3]) const { return const_cast<Tree3D*>(this)->Get(location); }

	void Set(const COMPONENT_T location[3], const T& value)
	{
		bool created;
		Node* node = _FindOrCreate(location, value);
		if (!created) node->value = value;
	}

	void Set(const COMPONENT_T location[3], T&& value)
	{
		bool created;
		Node* node = _FindOrCreate(location, std::move(value));
		if (!created) node->value = std::move(value);
	}

	template <typename F>
	requires Concepts::Function<F, void, T&>
	void ForEach(const F& function)
	{


	}
};
