#include "material.h"
#include "BRDF.h"
#include "Random.h"
#include <glm/gtc/constants.hpp>
#include <random>
#include <Walnut/Random.h>
#include <iostream>

bool material::scatter(const ray& rayIn, ray& rayOut, const hit_info& hitInfo, float& pdf) const
{
	glm::vec3 viewDirection = glm::normalize(-rayIn.direction);

	float dielectricF0 = 0.08f * specular;
	glm::vec3 F0 = glm::mix(glm::vec3(dielectricF0), baseColour, metallic);

	float dotNV = glm::clamp(glm::dot(hitInfo.worldNormal, viewDirection), 0.001f, 1.0f);
	float F = BRDF::fresnelSchlick(dotNV, glm::vec3(dielectricF0)).x;

	float specularChance = glm::mix(F, 1.0f, metallic);
	float diffuseChance = 1.0f - specularChance;

	if (Random::getReal(0.0f, 1.0f) < specularChance)
	{
		// specular lobe
		glm::vec3 halfVector = getHalfVector(hitInfo.worldNormal);
		glm::vec3 lightDirection = glm::reflect(-viewDirection, halfVector);

		float dotNL = glm::dot(hitInfo.worldNormal, lightDirection);
		if (dotNL <= 0.0f) return false;

		rayOut = { hitInfo.worldPosition + 0.001f * hitInfo.worldNormal, lightDirection };

		float dotNH = glm::clamp(glm::dot(hitInfo.worldNormal, halfVector), 0.0f, 1.0f);
		float dotVH = glm::clamp(glm::dot(viewDirection, halfVector), 0.0f, 1.0f);
		float D = BRDF::distributionGGX(dotNH, roughness);

		float specPDF = (D * dotNH) / (4.0f * dotVH);
		pdf = specPDF * specularChance;
	}
	else
	{
		// diffuse lobe
		glm::vec3 lightDirection = glm::normalize(hitInfo.worldNormal + Walnut::Random::InUnitSphere());

		if (glm::length(lightDirection) < 0.001f)
		{
			lightDirection = hitInfo.worldNormal;
		}

		rayOut.origin = hitInfo.worldPosition + 0.001f * hitInfo.worldNormal;
		rayOut.direction = glm::normalize(lightDirection);

		float cosTheta = glm::dot(hitInfo.worldNormal, rayOut.direction);
		float diffusePDF = cosTheta * glm::one_over_pi<float>();
		pdf = diffusePDF * diffuseChance;
	}

	return true;
}
 
glm::vec3 material::brdf(const glm::vec3& viewDirection, const glm::vec3& lightDirection, const glm::vec3& normal) const
{
	glm::vec3 halfVector = glm::normalize(viewDirection + lightDirection);
	float dotNV = glm::clamp(glm::dot(normal, viewDirection), 0.001f, 1.0f); // avoids 0 division
	float dotNL = glm::clamp(glm::dot(normal, lightDirection), 0.001f, 1.0f);
	float dotVH = glm::clamp(glm::dot(viewDirection, halfVector), 0.0f, 1.0f);
	float dotNH = glm::clamp(glm::dot(normal, halfVector), 0.0f, 1.0f);

	float dielectricF0 = 0.08f * specular;
	glm::vec3 F0 = glm::mix(glm::vec3(dielectricF0), baseColour, metallic);

	// specular
	float D = BRDF::distributionGGX(dotNH, roughness);
	glm::vec3 F = BRDF::fresnelSchlick(dotVH, F0);
	float G = BRDF::geometrySmith(glm::abs(dotNV), glm::abs(dotNL), roughness);

	glm::vec3 numerator = D * F * G;
	float denominator = 4.0f * glm::abs(dotNL) * glm::abs(dotNV);

	glm::vec3 specularComponent = numerator / denominator;

	// diffuse
	glm::vec3 kS = F;
	glm::vec3 kD = (glm::vec3(1.0f) - kS) * (1.0f - metallic);

	glm::vec3 lambert = baseColour * glm::one_over_pi<float>();
	float FL = glm::pow(1.0f - dotNL, 5);
	float FV = glm::pow(1.0f - dotNV, 5);
	float RR = 2.0f * roughness * (dotVH * dotVH);

	glm::vec3 retroReflection = lambert * RR * (FL + FV + FL * FV * (RR - 1.0f));
	glm::vec3 diffuseComponent = lambert * (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retroReflection;

	return kD * diffuseComponent + specularComponent;
}

glm::vec3 material::getHalfVector(const glm::vec3& normal) const
{
	float u1 = Random::getReal(0.0f, 1.0f);
	float u2 = Random::getReal(0.0f, 1.0f);

	return BRDF::sampleGGX(normal, roughness, u1, u2);
}
