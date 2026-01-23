#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace BRDF
{
	inline float distributionGGX(float cosTheta, float roughness)
	{
		float a = roughness * roughness;
		float a2 = a * a;
		float cosThetaSquared = cosTheta * cosTheta;
		float denominator = cosThetaSquared * (a2 - 1.0f) + 1.0f;
		denominator *= denominator * glm::pi<float>();

		return a2 / denominator;
	}

	inline glm::vec3 fresnelSchlick(float cosTheta, const glm::vec3 F0)
	{
		return F0 + (1.0f - F0) * glm::pow((1.0f - cosTheta), 5.0f);
	}

	inline float geometrySchlickGGXG1(float cosTheta, float k)
	{
		return cosTheta / (cosTheta * (1.0f - k) + k);
	}

	inline float geometrySmith(float dotNV, float dotNL, float roughness)
	{
		float remappedRoughness = 0.5f + roughness * 0.5f;
		float a = remappedRoughness * remappedRoughness;
		float k = a / 2.0f;

		return geometrySchlickGGXG1(dotNV, k) * geometrySchlickGGXG1(dotNL, k);
	}

	inline glm::vec3 sampleGGX(glm::vec3 normal, float roughness, float u1, float u2)
	{
		float a = roughness * roughness;
		float aSquared = a * a;

		float phi = 2.0f * glm::pi<float>() * u1;

		float cosTheta = glm::sqrt(glm::max(0.0f, (1.0f - u2) / (1.0f + (aSquared - 1.0f) * u2)));
		float sinTheta = glm::sqrt(glm::max(0.0f, 1.0f - cosTheta * cosTheta));

		glm::vec3 hLocal{ glm::cos(phi) * sinTheta, glm::sin(phi) * sinTheta, cosTheta };

		glm::vec3 helper = (glm::abs(normal.x) > 0.9f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

		glm::vec3 tangent = glm::normalize(glm::cross(helper, normal));
		glm::vec3 bitangent = glm::cross(normal, tangent);

		return glm::mat3(tangent, bitangent, normal) * hLocal;
	}
}