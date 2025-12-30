#include "material.h"
#include <Walnut/Random.h>
#include "Random.h"
#include <random>
#include <algorithm>

// Diffuse implementation
bool diffuse::scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const
{
	scattered_ray.origin = hit_info.world_position;
	scattered_ray.direction = hit_info.world_normal + diffuse_reflect(hit_info.world_normal);

	attenuation = albedo;
	return true;
}

glm::vec3 diffuse::diffuse_reflect(const glm::vec3& normal) const
{
	glm::vec3 vector = Walnut::Random::InUnitSphere();

	if (glm::dot(vector, normal) > 0.0f)
	{
		return vector;
	}
	else
	{
		return -vector;
	}
}

// Metal implementation
bool metal::scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const
{
	glm::vec3 reflection = reflect(current_ray.direction, hit_info.world_normal);
	scattered_ray.origin = hit_info.world_position;
	scattered_ray.direction = normalize(reflection + roughness * Walnut::Random::Vec3(-1.0f, 1.0f));

	attenuation = albedo;
	return true;
}

// Dielectric implementation
bool dielectric::scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const
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
	}

	float abs_cos_i = std::abs(cos_i);
	float sin_i = std::sqrt(1 - abs_cos_i * abs_cos_i); // sine of incident angle
	float eta = n1 / n2;
	float reflectance = calculate_reflectance(abs_cos_i, n1, n2);

	if (eta * sin_i > 1.0f || reflectance >= Random::getReal<float>(0.0f, 1.0f))
	{
		glm::vec3 reflection = reflect(current_ray.direction, normal);
		scattered_ray.direction = normalize(reflection);
	}
	else
	{
		glm::vec3 refracted_perp = eta * (current_ray.direction + abs_cos_i * normal);
		glm::vec3 refracted_parallel = -std::sqrt(std::fabs(1.0f - glm::dot(refracted_perp, refracted_perp))) * normal;
		scattered_ray.direction = refracted_perp + refracted_parallel;
	}

	scattered_ray.origin = hit_info.world_position;
	attenuation = albedo;
	return true;
}

double dielectric::calculate_reflectance(const float& cos_i, const float& n1, const float& n2) const
{
	float base_reflectance = (n1 - n2) / (n1 + n2);
	base_reflectance *= base_reflectance;
	return base_reflectance + (1.0f - base_reflectance) * pow((1.0f - cos_i), 5);
}