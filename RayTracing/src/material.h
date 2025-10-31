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
		glm::vec3 normal = hit_info.world_normal;
		float n1 = 1.0f;
		float n2 = refractive_index;
		float cos_theta = fmin(dot(current_ray.direction, normal), 1.0f);

		if (cos_theta > 0.0f)
		{
			normal = -normal;
			std::swap(n1, n2);
			cos_theta = -cos_theta;
		}

		float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
		float eta = n1 / n2;

		if (eta * sin_theta > 1.0f)
		{
			glm::vec3 reflection = reflect(current_ray.direction, normal);
			scattered_ray.direction = normalize(reflection);
		}
		else
		{
			cos_theta = fmin(dot(-current_ray.direction, normal), 1.0f);
			glm::vec3 refracted_perp = eta * (current_ray.direction + cos_theta * normal);
			glm::vec3 refracted_parallel = -std::sqrt(std::fabs(1.0f - glm::dot(refracted_perp, refracted_perp))) * normal;
			scattered_ray.direction = refracted_perp + refracted_parallel;
		}

		float offset_dir = (glm::dot(scattered_ray.direction, normal) > 0.0f) ? 1.0f : -1.0f;
		scattered_ray.origin = hit_info.world_position + 0.001f * offset_dir * normal;
		attenuation = albedo;
		return true;
	}
};