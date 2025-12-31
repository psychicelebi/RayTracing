#pragma once
#include <vector>
#include "object.h"
#include <memory>
#include <execution>

struct BVHNode
{
	extent bounds;
	std::vector<int> object_indices;
	std::array<std::unique_ptr<BVHNode>, 8> children;

	bool is_leaf()
	{
		for (auto& child : children)
		{
			if (child)
			{
				return false;
			}
		}
		return true;
	}
};

class BVH
{
public:
	std::unique_ptr<BVHNode> root;

	const int MAX_OBJECTS = 2;

	BVH(const std::vector<std::unique_ptr<object>>& objects);

private:
	void build_tree(BVHNode* node, const std::vector<std::unique_ptr<object>>& objects, const std::vector<int>& object_indices);

	extent calculate_child_bounds(const extent& parent_bounds, int index) const;

	bool in_volume(const extent& bounds, const object* objects) const;
};