#pragma once
#include <vector>
#include "object.h"
#include <memory>
#include <execution>

struct BVHNode
{
	extent bounds;
	std::vector<int> object_indices;
	std::vector<std::unique_ptr<BVHNode>> children;
};

class BVH
{
public:
	BVH(std::vector<std::unique_ptr<object>> objects)
	{
		extent scene_aabb{};

		for (size_t i = 0; i < objects.size(); i++)
		{
			scene_aabb.expand(objects[i].get()->get_extent({ 0,1,2 }));
		}

		BVHNode node{ scene_aabb };
		root = std::make_unique<BVHNode>(node);
	}

private:
	std::unique_ptr<BVHNode> root;
	void build_tree(BVHNode* node, extent bounds, std::vector<int>& object_indices, const std::vector<std::unique_ptr<object>>& objects)
	{
		size_t i = 0;
		const int max_objects = 2;

		while (i < objects.size())
		{
			for (int i = 0; i < 7; i++)
			{
				if (bounds.active[i])
				{
					glm::vec3 normal = extent::plane_set_normals[i];
					
					float distance = glm::dot(objects[i].get()->position, normal);

					if (distance > bounds.slabs[i].d_near && distance < bounds.slabs[i].d_far)
					{
						node->object_indices.push_back(i);
					}
				}
			}
		}
	}
};