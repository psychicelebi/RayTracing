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

// Dielectric implementation
bool dielectric::scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const
{
	glm::vec3 viewDirection = -rayIn.direction;
	glm::vec3 normal = hitInfo.worldNormal;

	float dotNV = glm::dot(normal, viewDirection);

	float eta = dotNV > 0.0f ? (1.0f / refractiveIndex) : refractiveIndex;
	glm::vec3 orientedNormal = dotNV > 0.0f ? normal : -normal;

	glm::vec3 halfVector = getHalfVector(orientedNormal);
	float dotVH = glm::clamp(glm::dot(viewDirection, halfVector), 0.001f, 1.0f);
	float dotNH = glm::clamp(glm::dot(orientedNormal, halfVector), 0.001f, 1.0f);

	float F0 = (refractiveIndex - 1.0f) / (refractiveIndex + 1.0f);
	F0 *= F0;
	float F = BRDF::fresnelSchlick(dotVH, glm::vec3(F0)).x;
	float D = BRDF::distributionGGX(dotNH, roughness);

	glm::vec3 lightDirection = glm::refract(-viewDirection, halfVector, eta);

	if (glm::length(lightDirection) < 0.001f || F >= Random::getReal<float>(0.0f, 1.0f))
	{
		lightDirection = glm::reflect(-viewDirection, halfVector);
		rayOut.direction = normalize(lightDirection);

		pdf = (D * dotNH) / (4.0f * dotVH);
	}
	else
	{
		rayOut.direction = normalize(lightDirection);

		float dotLH = glm::dot(lightDirection, halfVector);
		float numerator = D * dotNH * glm::abs(dotLH) * eta * eta;
		float sqrtDenominator = dotVH + eta * dotLH;
		sqrtDenominator = glm::max(glm::abs(sqrtDenominator), 0.001f) * glm::sign(sqrtDenominator);
		pdf = numerator / (sqrtDenominator * sqrtDenominator);
	}

	rayOut.origin = hitInfo.worldPosition + (glm::dot(rayOut.direction, orientedNormal) > 0.0f ? 0.001f : -0.001f) * hitInfo.worldNormal;
	pdf = glm::max(pdf, 0.0001f);
	return true;
}

glm::vec3 dielectric::brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	float dotNV = glm::dot(normal, viewDirection);
	float dotNL = glm::dot(normal, lightDirection);

	bool isReflection = dotNV * dotNL > 0.0f;

	float eta = dotNV > 0.0f ? (1.0f / refractiveIndex) : refractiveIndex;
	glm::vec3 orientedNormal = dotNV > 0.0f ? normal : -normal;

	glm::vec3 halfVector{};
	if (isReflection)
	{
		halfVector = glm::normalize(viewDirection + lightDirection);
	}
	else
	{
		halfVector = glm::normalize(viewDirection + lightDirection * eta);
	}

	if (glm::dot(halfVector, orientedNormal) < 0.0f)
	{
		halfVector = -halfVector;
	}

	float absNV = glm::max(glm::abs(dotNV), 0.001f);
	float absNL = glm::max(glm::abs(dotNL), 0.001f);
	float dotNH = glm::clamp(glm::dot(orientedNormal, halfVector), 0.001f, 1.0f);
	float dotVH = glm::clamp(glm::dot(viewDirection, halfVector), 0.001f, 1.0f);
	float dotLH = glm::dot(lightDirection, halfVector);

	float D = BRDF::distributionGGX(dotNH, roughness);
	float G = BRDF::geometrySmith(absNV, absNL, roughness);
	float F0 = (refractiveIndex - 1.0f) / (refractiveIndex + 1.0f);
	F0 *= F0;
	float F = BRDF::fresnelSchlick(dotVH, glm::vec3(F0)).x;

	if (isReflection)
	{
		float numerator = D * F * G;
		float denominator = 4.0f * absNV * absNL;

		return glm::vec3(numerator / denominator);
	}
	else
	{
		float numerator = glm::abs(dotVH) * glm::abs(dotLH) * (1.0f - F) * D * G;
		float sqrtDenominator = dotVH + eta * dotLH;
		sqrtDenominator = glm::max(glm::abs(sqrtDenominator), 0.001f) * glm::sign(sqrtDenominator);
		float denominator = (sqrtDenominator * sqrtDenominator) * absNV * absNL;
		denominator = glm::max(denominator, 0.00001f);

		float brdfValue = (1.0f / (eta * eta)) * numerator / denominator;
		brdfValue = glm::min(brdfValue, 50.0f);

		if (Random::getReal(0.0f, 1.0f) < 0.001f) {
			std::cout << "BRDF refraction: " << brdfValue << ", eta: " << eta << std::endl;
		}


		return glm::vec3(brdfValue);
	}
}

glm::vec3 material::getHalfVector(const glm::vec3& normal) const
{
	float u1 = Random::getReal(0.0f, 1.0f);
	float u2 = Random::getReal(0.0f, 1.0f);

	return BRDF::sampleGGX(normal, roughness, u1, u2);
}
