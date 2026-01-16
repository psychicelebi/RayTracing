#pragma once
#include <glm/glm.hpp>

namespace BRDF
{
	inline float distributionGGX(float cosTheta, float roughness)
	{
		float roughnessSquared = roughness * roughness;
		float cosThetaSquared = cosTheta * cosTheta;
		float denominator = cosThetaSquared * (roughnessSquared - 1.0f) + 1.0f;

		const float c = (roughnessSquared)*glm::one_over_pi<float>();
		return c / (denominator * denominator);
	}

	inline glm::vec3 fresnelSchlick(float cosTheta, const glm::vec3 baseColour)
	{
		return baseColour + (1.0f - baseColour) * glm::pow((1 - cosTheta), 5.0f);
	}

	inline float geometrySchlickGGXG1(float cosTheta, float k)
	{
		return cosTheta / (cosTheta * (1 - k) + k);
	}

	inline float geometrySmith(float dotNV, float dotNL, float roughness)
	{
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		return geometrySchlickGGXG1(dotNV, k) * geometrySchlickGGXG1(dotNL, k);
	}

	inline glm::vec3 sampleGGX(glm::vec3 normal, float roughness, float u1, float u2)
	{
		float a = roughness * roughness;
		float aSquared = a * a;

		float phi = 2.0f * 3.14159265f * u1;

		float cosTheta = sqrt(glm::max(0.0f, (1.0f - u2) / (1.0f + (aSquared - 1.0f) * u2)));
		float sinTheta = sqrt(glm::max(0.0f, 1.0f - cosTheta * cosTheta));

		glm::vec3 hLocal;
		hLocal.x = cos(phi) * sinTheta;
		hLocal.y = sin(phi) * sinTheta;
		hLocal.z = cosTheta;


		glm::vec3 up = abs(normal.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
		glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
		glm::vec3 bitangent = glm::cross(normal, tangent);

		return tangent * hLocal.x + bitangent * hLocal.y + normal * hLocal.z;
	}
}