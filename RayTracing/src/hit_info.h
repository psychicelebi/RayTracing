#pragma once
#include <glm/glm.hpp>

struct hit_info
{
	float hitDistance;
	glm::vec3 worldPosition;
	glm::vec3 worldNormal;

	int objectIndex;
};