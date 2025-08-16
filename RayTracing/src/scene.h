#pragma once
#include "material.h"

#include <glm/glm.hpp>
#include <vector>

struct sphere 
{
	glm::vec3 centre{0.0f, 0.0f, -2.0f};
	float radius = 0.5f;
	int material_index = 0;
};

struct scene 
{
	std::vector<sphere> spheres;
	glm::vec3 light_direction{ -1.0f, -1.0f, -1.0f };
	std::vector<material> materials;
};