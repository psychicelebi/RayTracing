#pragma once
#include <glm/glm.hpp>
#include <Walnut/Random.h>
#include "ray.h"
#include "hit_info.h"
#include "Random.h"


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
	glm::vec3 diffuse_reflect(const glm::vec3& normal) const 
	{
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
	float refractive_index = 1.5f;

	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override
	{
		static std::mt19937 rng{ std::random_device{}() };
		static std::uniform_real_distribution<float> probability(0.0f, 1.0f);

		glm::vec3 normal = hit_info.world_normal;
		float n1 = 1.0f;
		float n2 = refractive_index;
		float cos_i = fmin(dot(current_ray.direction, normal), 1.0f); // cosine of incident angle

		if (cos_i > 0.0f)
		{
			normal = -normal;
			std::swap(n1, n2);
			cos_i = -cos_i;
		}

		float sin_i = std::sqrt(1 - cos_i * cos_i); // sine of incident angle
		float eta = n1 / n2;
		float reflectance = calculate_reflectance(std::fabs(cos_i), n1, n2);

		if (eta * sin_i > 1.0f || reflectance >= Random::getReal<float>(0.0f, 1.0f))
		{
			glm::vec3 reflection = reflect(current_ray.direction, normal);
			scattered_ray.direction = normalize(reflection);
		}
		else
		{
			cos_i = fmin(dot(-current_ray.direction, normal), 1.0f);
			glm::vec3 refracted_perp = eta * (current_ray.direction + cos_i * normal);
			glm::vec3 refracted_parallel = -std::sqrt(std::fabs(1.0f - glm::dot(refracted_perp, refracted_perp))) * normal;
			scattered_ray.direction = refracted_perp + refracted_parallel;
		}

		float offset_dir = (glm::dot(scattered_ray.direction, normal) > 0.0f) ? 1.0f : -1.0f;
		scattered_ray.origin = hit_info.world_position + 0.001f * offset_dir * normal;
		attenuation = albedo;
		return true;
	}

private:
	double calculate_reflectance(const float& cos_i, const float& n1, const float& n2) const
	{
		float base_reflectance = (n1 - n2) / (n1 + n2);
		base_reflectance *= base_reflectance;
		return base_reflectance + (1.0f - base_reflectance) * pow((1.0f - cos_i), 5);
	}
};