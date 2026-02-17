#pragma once
#include "hit_info.h"
#include "ray.h"

class material
{
public:
	glm::vec3 baseColour{ 1.0f };
	float roughness{ 0.5f };
	float metallic{ 0.5f };
	float specular{ 0.5f };


	virtual ~material() {}

	virtual bool scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const;

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3& lightDirection, const glm::vec3& normal) const;

	virtual glm::vec3 emitted() const { return glm::vec3(0.0f); }

	glm::vec3 getHalfVector(const glm::vec3& normal, const glm::vec3& viewDirection) const;

};

class emissive : public material 
{
public:
	float emissionStrength{ 1.0f };

	bool scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const override { return false; }

	virtual glm::vec3 brdf(const glm::vec3& viewDirection, const glm::vec3& lightDirection, const glm::vec3& normal) const override { return glm::vec3(0.0f); }

	virtual glm::vec3 emitted() const override { return baseColour * emissionStrength; }

};
