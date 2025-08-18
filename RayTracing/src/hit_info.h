#pragma once
#include <glm/glm.hpp>

struct hit_info
{
	float hit_distance;
	glm::vec3 world_position;
	glm::vec3 world_normal;

	int object_index;
};