#pragma once

#include <glm/glm.hpp>
#include <vector>


struct sphere {
	glm::vec3 centre{0.0f};
	float radius = 0.5f;
	glm::vec3 albedo{ 1.0f };
};

struct scene {
	std::vector<sphere> spheres;
};