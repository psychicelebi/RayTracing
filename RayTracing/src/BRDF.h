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
}