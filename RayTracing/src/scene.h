#pragma once
#include "material.h"

#include <glm/glm.hpp>
#include <vector>
#include "light.h"

struct sphere 
{
	glm::vec3 centre{0.0f, 0.0f, -2.0f};
	float radius = 0.5f;
	int material_index = 0;
};

struct scene 
{
	std::vector<sphere> spheres;
	std::unique_ptr<light> light;
	std::vector<std::unique_ptr<material>> materials;
	glm::vec3 background_colour{ 0.6f, 0.7f, 0.9f };
};