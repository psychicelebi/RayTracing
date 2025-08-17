#pragma once
#include <glm/glm.hpp>
#include <Walnut/Random.h>
#include "ray.h"
#include "hit_info.h"


class material
{
public:
	glm::vec3 albedo{ 1.0f };

	virtual glm::vec3 scatter(hit_info& hit_info, ray& current_ray) const = 0;
};

class diffuse : public material
{
public:
	glm::vec3 scatter(hit_info& hit_info, ray& current_ray) const 
	{
		return albedo;
	}
};

class metal : public material 
{
public:
	float roughness = 0.0f;

	glm::vec3 scatter(hit_info& hit_info, ray& current_ray) const override
	{
		current_ray.origin = hit_info.world_position + hit_info.world_normal * 0.001f;
		current_ray.direction = reflect(current_ray.direction, hit_info.world_normal + roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

		return albedo;
	}
};