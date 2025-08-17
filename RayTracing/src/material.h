#pragma once
#include <glm/glm.hpp>
#include <Walnut/Random.h>
#include "ray.h"


class material
{
public:
	glm::vec3 albedo{ 1.0f };

	virtual ~material() {}

	virtual glm::vec3 scatter(const glm::vec3& world_position, const glm::vec3& world_normal, ray& current_ray) const = 0;
};

class diffuse : public material
{
public:
	glm::vec3 scatter(const glm::vec3& world_position, const glm::vec3& world_normal, ray& current_ray) const
	{
		return albedo;
	}
};

class metal : public material 
{
public:
	float roughness = 0.0f;

	glm::vec3 scatter(const glm::vec3& world_position, const glm::vec3& world_normal, ray& current_ray) const override
	{
		current_ray.origin = world_position + world_normal * 0.001f;
		current_ray.direction = reflect(current_ray.direction, world_normal + roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

		return albedo * 0.8f;
	}
};

class dielectric : public material
{
public:
	float refractive_index = 1.0f;

	glm::vec3 scatter(const glm::vec3& world_position, const glm::vec3& world_normal, ray& current_ray) const
	{
		return albedo;
	}
};