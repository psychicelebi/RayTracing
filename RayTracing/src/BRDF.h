#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace BRDF
{
	inline float distributionGGX(float dotNH, float roughness)
	{
		float a = roughness;
		float a2 = a * a;
		float dotNHSquared = dotNH * dotNH;
		float denominator = dotNHSquared * (a2 - 1.0f) + 1.0f;
		denominator *= denominator;

		return a2 / (glm::pi<float>() * denominator);
	}

	inline glm::vec3 fresnelSchlick(float cosTheta, const glm::vec3 F0)
	{
		float x = glm::clamp(1.0f - cosTheta, 0.0f, 1.0f);
		float x2 = x * x;
		return F0 + (1.0f - F0) * (x2 * x2 * x);
	}

	inline float geometrySchlickGGXG1(float cosTheta, float a2)
	{
		float denominator = cosTheta + glm::sqrt(a2 + (1.0f - a2) * (cosTheta * cosTheta));
		return 2.0f * cosTheta / denominator;
	}

	inline float geometrySmith(float dotNV, float dotNL, float roughness)
	{
		float alpha = roughness;
		float alpha2 = alpha * alpha;

		float a = dotNV * glm::sqrt(alpha2 + dotNL * (dotNL - alpha2 * dotNL));
		float b = dotNL * glm::sqrt(alpha2 + dotNV * (dotNV - alpha2 * dotNV));

		return 0.5f / (a + b);

		return geometrySchlickGGXG1(dotNV, alpha2) * geometrySchlickGGXG1(dotNL, alpha2);
	}

	inline glm::vec3 sampleGGX(glm::vec3 normal, float roughness, float u1, float u2)
	{
		float a = roughness;
		float a2 = a * a;

		float phi = 2.0f * glm::pi<float>() * u1;

		float cosTheta = glm::sqrt(glm::max(0.0f, (1.0f - u2) / (1.0f + (a2 - 1.0f) * u2)));
		float sinTheta = glm::sqrt(glm::max(0.0f, 1.0f - cosTheta * cosTheta));

		glm::vec3 hLocal{ glm::cos(phi) * sinTheta, glm::sin(phi) * sinTheta, cosTheta };

		glm::vec3 helper = (glm::abs(normal.x) > 0.9f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

		glm::vec3 tangent = glm::normalize(glm::cross(helper, normal));
		glm::vec3 bitangent = glm::cross(normal, tangent);

		return glm::mat3(tangent, bitangent, normal) * hLocal;
	}
}