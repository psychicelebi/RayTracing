#pragma once
#include "hit_info.h"
#include "Random.h"
#include "ray.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <Walnut/Random.h>

class material
{
public:
	glm::vec3 baseColour{ 1.0f };
	float roughness{ 0.5f };


	virtual ~material() {}

	virtual bool scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, float& pdf) const = 0;

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const = 0;

	virtual glm::vec3 emitted() const { return glm::vec3(0.0f); }

};

class diffuse : public material
{
public:
	bool scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, float& pdf) const override;

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const override;
};

class metal : public material
{
public:

	bool scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, float& pdf) const override;

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const override;
};

class emissive : public material 
{
public:
	float emissionStrength{ 1.0f };

	bool scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, float& pdf) const override { return false; }

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const override { return glm::vec3(0.0f); }

	virtual glm::vec3 emitted() const override { return baseColour * emissionStrength; }

};

/*
class dielectric : public material
{
public:
	float refractive_index = 1.5f;

	bool scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, float& pdf) const override;

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const override;

private:
	double calculate_reflectance(const float cos_i, const float n1, const float n2) const;
};
*/