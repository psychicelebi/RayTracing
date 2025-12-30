#pragma once
#include <glm/glm.hpp>
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
	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override;

private:
	glm::vec3 diffuse_reflect(const glm::vec3& normal) const;
};

class metal : public material
{
public:
	float roughness = 0.0f;

	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override;
};

class dielectric : public material
{
public:
	float refractive_index = 1.5f;

	bool scatter(ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const override;

private:
	double calculate_reflectance(const float& cos_i, const float& n1, const float& n2) const;
};