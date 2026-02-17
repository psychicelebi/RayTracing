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

	glm::vec3 F0 = glm::mix(glm::vec3(0.08f * specular), baseColour, metallic);

	float dotNV = glm::max(0.0f, glm::dot(hitInfo.worldNormal, viewDirection));
	glm::vec3 fVec = BRDF::fresnelSchlick(dotNV, F0);
	float F = (fVec.r + fVec.g + fVec.b) / 3.0f;

	float specularWeight = F;
	float diffuseWeight = (1.0f - F) * (1.0f - metallic);
	float totalWeight = specularWeight + diffuseWeight;

	float specularChance = specularWeight / totalWeight;
	float diffuseChance = diffuseWeight / totalWeight;

	if (Random::getReal(0.0f, 1.0f) < specularChance)
	{
		// specular lobe
		glm::vec3 halfVector = getHalfVector(hitInfo.worldNormal, viewDirection);
		glm::vec3 lightDirection = glm::reflect(-viewDirection, halfVector);

		float dotNL = glm::dot(hitInfo.worldNormal, lightDirection);
		if (dotNL <= 0.0f) return false;

		rayOut = { hitInfo.worldPosition + 0.001f * hitInfo.worldNormal, lightDirection };

		float dotNH = glm::max(0.0f, glm::dot(hitInfo.worldNormal, halfVector));
		float dotLH = glm::max(0.0f, glm::dot(lightDirection, halfVector));
		dotNV = glm::max(1e-5f, dotNV);

		float D = BRDF::distributionGGX(dotNH, roughness);
		float G1 = BRDF::geometrySchlickGGXG1(dotNV, roughness * roughness);

		float specPDF = (D * G1) / (4.0f * dotNV);
		pdf = specPDF * specularChance;
	}
	else
	{
		// diffuse lobe
		glm::vec3 lightDirection = glm::normalize(hitInfo.worldNormal + Walnut::Random::InUnitSphere());

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

	float dotNV = glm::max(0.0f, glm::dot(normal, viewDirection)); // avoids 0 division
	float dotNL = glm::max(0.0f, glm::dot(normal, lightDirection));
	float dotVH = glm::max(0.0f, glm::dot(viewDirection, halfVector));
	float dotNH = glm::max(0.0f, glm::dot(normal, halfVector));

	glm::vec3 F0 = glm::mix(glm::vec3(0.08f * specular), baseColour, metallic);

	// specular
	float D = BRDF::distributionGGX(dotNH, roughness);
	glm::vec3 F = BRDF::fresnelSchlick(dotVH, F0);
	float G = BRDF::geometrySmith(dotNV, dotNL, roughness);

	glm::vec3 specularComponent = D * F * G; // denominator 4.0f * dotNL * dotNV baked into G term;

	// diffuse
	glm::vec3 lambert = baseColour * glm::one_over_pi<float>();
	float FL = pow(1.0f - dotNL, 5.0f);
	float FV = pow(1.0f - dotNV, 5.0f);
	float Fd90 = 0.5f + 2.0f * roughness * (dotVH * dotVH);

	glm::vec3 kS = F;
	glm::vec3 kD = (glm::vec3(1.0f) - kS) * (1.0f - metallic);
	glm::vec3 diffuseComponent = kD * lambert * glm::mix(1.0f, Fd90, FL) * glm::mix(1.0f, Fd90, FV);

	return diffuseComponent + specularComponent;
}

glm::vec3 material::getHalfVector(const glm::vec3& normal, const glm::vec3& viewDirection) const
{
	float u1 = Random::getReal(0.0f, 1.0f);
	float u2 = Random::getReal(0.0f, 1.0f);

	return BRDF::sampleGGXVNDF(normal,viewDirection, roughness, u1, u2);
}
