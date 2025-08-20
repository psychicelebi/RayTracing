#pragma once
#include <glm/glm.hpp>
#include <Walnut/Random.h>
#include "ray.h"
#include "hit_info.h"


class material
{
public:
	glm::vec3 albedo{ 1.0f };

	virtual ~material() {}

	virtual bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const = 0;
};

class diffuse : public material
{
public:
	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override
	{
		scattered_ray.origin = hit_info.world_position + 0.001f * hit_info.world_normal;
		scattered_ray.direction = hit_info.world_normal + diffuse_reflect(hit_info.world_normal);

		attenuation = albedo;
		return true;
	}

private:
	glm::vec3 diffuse_reflect(const glm::vec3& normal) const {
		glm::vec3 vector{ 0.0f };
		do
		{
			vector = Walnut::Random::Vec3(-1.0f, 1.0f);
		} while (glm::length(vector) > 1.0f);

		vector = normalize(vector);

		if (glm::dot(vector, normal) > 0.0f)
		{
			return vector;
		}
		else 
		{
			return -vector;
		}
	}
};

class metal : public material 
{
public:
	float roughness = 0.0f;

	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override
	{
		glm::vec3 reflection = reflect(current_ray.direction, hit_info.world_normal);
		scattered_ray.origin = hit_info.world_position + 0.001f * hit_info.world_normal;
		scattered_ray.direction = normalize(reflection + roughness * Walnut::Random::Vec3(-1.0f, 1.0f));

		attenuation = albedo;
		return true;
	}
};

class dielectric : public material
{
public:
	float refractive_index = 1.0f;

	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override
	{
		return true;
	}
};