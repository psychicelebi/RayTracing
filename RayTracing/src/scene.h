#pragma once
#include "material.h"
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include "light.h"
#include "object.h"

struct scene 
{
	std::vector<std::unique_ptr<object>> objects{};
	std::vector<std::unique_ptr<light>> lights{};
	std::vector<std::unique_ptr<material>> materials{};
	glm::vec3 background_colour{ 0.6f, 0.7f, 0.9f };

	// std::unique_ptr<BVH> bvh;
};