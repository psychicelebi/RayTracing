#include "scene.h"

hit_info scene::traceRay(const ray& ray) const
{
	if (objects.size() == 0)
	{
		return hit_info{};
	}

	int closestObjectIndex = -1;
	float closestT = FLT_MAX;
	constexpr float T_MIN = 0.001f; // to avoid self-intersection

	std::priority_queue<std::pair<float, BVHNode*>,
		std::vector<std::pair<float, BVHNode*>>,
		std::greater<>> candidateNodes{};

	BVHNode* root = bvh.get()->root.get();
	float rootT = root->bounds.hit(ray);

	if (rootT >= 0.0f)
	{
		candidateNodes.push({ rootT, root });
	}

	while (!candidateNodes.empty())
	{
		auto [currentNodeT, currentNode] = candidateNodes.top();
		candidateNodes.pop();

		if (currentNodeT < closestT)
		{
			if (currentNode->is_leaf())
			{
				for (int index : currentNode->object_indices)
				{
					float t = objects[index]->hit(ray);

					if (t >= T_MIN && t < closestT)
					{
						closestT = t;
						closestObjectIndex = index;
					}
				}
			}
			else
			{
				for (auto& child : currentNode->children)
				{
					float t = child.get()->bounds.hit(ray);

					if (t >= 0.0f)
					{
						candidateNodes.push({ t, child.get() });
					}
				}
			}
		}
		else
		{
			break;
		}
	}

	if (closestObjectIndex < 0)
	{
		return hit_info();
	}

	return makeHit(ray, closestObjectIndex, closestT);
}

hit_info scene::makeHit(const ray& ray, int objectIndex, float hitDistance) const
{
	hit_info hitInfo{};
	hitInfo.hitDistance = hitDistance;
	hitInfo.objectIndex = objectIndex;


	const object* closestObject = objects[objectIndex].get();

	hitInfo.materialIndex = closestObject->material_index;
	hitInfo.worldPosition = ray.origin + hitDistance * ray.direction;
	hitInfo.worldNormal = closestObject->getNormalAt(hitInfo.worldPosition);

	return hitInfo;
}
