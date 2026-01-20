#include "material.h"
#include "BRDF.h"
#include "Random.h"
#include <glm/gtc/constants.hpp>
#include <random>
#include <Walnut/Random.h>
#include <iostream>

// Diffuse implementation
bool diffuse::scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const
{
	glm::vec3 lightDirection = glm::normalize(hitInfo.worldNormal + Walnut::Random::InUnitSphere());

	if (glm::length(lightDirection) < 0.001f )
	{
		lightDirection = hitInfo.worldNormal;
	}

	rayOut.origin = hitInfo.worldPosition + 0.001f * hitInfo.worldNormal;
	rayOut.direction = glm::normalize(lightDirection);

	float cosTheta = glm::dot(hitInfo.worldNormal, rayOut.direction);

	pdf = cosTheta * glm::one_over_pi<float>();
	return true;
}

glm::vec3 diffuse::brdf(const glm::vec3& rayDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	return baseColour * glm::one_over_pi<float>();
}

// Metal implementation
bool metal::scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const
{
	glm::vec3 viewDirection = glm::normalize(-rayIn.direction);
	glm::vec3 halfVector = getHalfVector(hitInfo.worldNormal);
	glm::vec3 lightDirection = glm::reflect(-viewDirection, halfVector);

	float dotNL = glm::dot(hitInfo.worldNormal, lightDirection);
	if (dotNL <= 0.0f) return false;

	rayOut = { hitInfo.worldPosition + 0.001f * hitInfo.worldNormal, lightDirection };

	float dotNH = glm::max(glm::dot(hitInfo.worldNormal, halfVector), 0.0f);
	float dotVH = glm::max(glm::dot(viewDirection, halfVector), 0.0f);
	float D = BRDF::distributionGGX(dotNH, roughness);

	pdf = (D * dotNH) / (4.0f * dotVH);
	return true;
}

glm::vec3 metal::brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	glm::vec3 halfVector = glm::normalize(viewDirection + lightDirection);

	float dotNV = glm::clamp(glm::dot(normal, viewDirection), 0.001f, 1.0f); // avoids 0 division
	float dotNL = glm::clamp(glm::dot(normal, lightDirection), 0.001f, 1.0f);
	float dotNH = glm::clamp(glm::dot(normal, halfVector), 0.0f, 1.0f);
	float dotVH = glm::clamp(glm::dot(viewDirection, halfVector), 0.0f, 1.0f);

	float D = BRDF::distributionGGX(dotNH, roughness);
	glm::vec3 F = BRDF::fresnelSchlick(dotVH, baseColour);
	float G = BRDF::geometrySmith(glm::abs(dotNV), glm::abs(dotNL), roughness);

	glm::vec3 numerator = D * F * G;
	float denominator = 4.0f * glm::abs(dotNL) * glm::abs(dotNV);

	return numerator / denominator;
}

glm::vec3 material::getHalfVector(const glm::vec3& normal) const
{
	float u1 = Random::getReal(0.0f, 1.0f);
	float u2 = Random::getReal(0.0f, 1.0f);

	return BRDF::sampleGGX(normal, roughness, u1, u2);
}
