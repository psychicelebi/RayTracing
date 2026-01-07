#pragma once
#include <glm/glm.hpp>

struct hit_info
{
	float hitDistance{ -1 };
	glm::vec3 worldPosition{};
	glm::vec3 worldNormal{};

	int objectIndex{ -1 };
	int materialIndex{ -1 };

	bool didHit() const { return hitDistance > 0.0f; }
};