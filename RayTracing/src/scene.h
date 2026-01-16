#pragma once
#include "material.h"
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include "light.h"
#include "object.h"
#include "BVH.h"

class scene 
{
public:
	std::vector<std::unique_ptr<object>> objects{};
	// std::vector<std::unique_ptr<light>> lights{};
	std::vector<std::unique_ptr<material>> materials{};
	glm::vec3 backgroundColour{ 0.6f, 0.7f, 0.9f };

	std::unique_ptr<BVH> bvh{};

	hit_info traceRay(const ray& ray) const;
	static glm::vec3 getSkyColour(const ray& ray);

private:
	hit_info makeHit(const ray& ray, int objectIndex, float hitDistance) const;
};