#include "BVH.h"

BVH::BVH(const std::vector<std::unique_ptr<object>>& objects)
{
	extent scene_aabb{};
	std::vector<int> all_indices(objects.size());

	for (size_t i = 0; i < objects.size(); i++)
	{
		scene_aabb.expand(objects[i].get()->get_extent({ 0,1,2 }));
		all_indices[i] = i;
	}

	root = std::make_unique<BVHNode>();
	root.get()->bounds = scene_aabb;

	build_tree(root.get(), objects, all_indices);
}

void BVH::build_tree(BVHNode* node, const std::vector<std::unique_ptr<object>>& objects, const std::vector<int>& object_indices)
{
	node->object_indices = object_indices;

	if (node->object_indices.size() > MAX_OBJECTS)
	{
		std::array<std::vector<int>, 8> child_object_indices{};

		for (int i = 0; i < 8; i++)
		{
			node->children[i] = std::make_unique<BVHNode>();
			node->children[i].get()->bounds = calculate_child_bounds(node->bounds, i);
		}
		
		for (int index : node->object_indices)
		{
			for (int i = 0; i < 8; i++)
			{
				if (in_volume(node->children[i].get()->bounds, objects[index].get()))
				{
					child_object_indices[i].push_back(index);
					break;
				}
			}
		}

		for (int i = 0; i < 8; i++)
		{
			build_tree(node->children[i].get(), objects, child_object_indices[i]);
		}

		node->object_indices.clear();
	}
	else 
	{
		// recalculate tight final bounds for node's objects
		if (!object_indices.empty()) 
		{
			node->bounds = objects[object_indices[0]]->get_extent({ 3,4,5,6 });
			for (size_t i = 1; i < object_indices.size(); i++) {
				node->bounds.expand(objects[object_indices[i]]->get_extent({ 3,4,5,6 }));
			}
		}
	}
}

extent BVH::calculate_child_bounds(const extent& parent_bounds, int index) const
{
	extent child_bounds = parent_bounds;

	for (int i = 0; i < 3; i++)
	{
		float d_near = parent_bounds.slabs[i].d_near;
		float d_far = parent_bounds.slabs[i].d_far;
		float midpoint = (d_near + d_far) / 2.0f;

		if (index & (1 << i))
		{
			child_bounds.slabs[i].d_near = midpoint;
		}
		else
		{
			child_bounds.slabs[i].d_far = midpoint;
		}
	}

	return child_bounds;
}

bool BVH::in_volume(const extent& bounds, const object* object) const
{
	for (int i = 0; i < 3; i++)
	{
		glm::vec3 normal = extent::plane_set_normals[i];

		float distance = glm::dot(object->position, normal);

		if (distance < bounds.slabs[i].d_near || distance > bounds.slabs[i].d_far)
		{
			return false;
		}
	}

	return true;

}
