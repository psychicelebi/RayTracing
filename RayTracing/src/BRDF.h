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

	inline glm::vec3 sampleGGXVNDF(glm::vec3 normal, glm::vec3 viewDirection, float roughness, float u1, float u2)
	{
		glm::vec3 helper = (glm::abs(normal.x) > 0.9f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 tangent = glm::normalize(glm::cross(helper, normal));
		glm::vec3 bitangent = glm::cross(normal, tangent);
		glm::mat3 TBN = glm::mat3(tangent, bitangent, normal);

		glm::vec3 ve = glm::transpose(TBN) * viewDirection;
		glm::vec3 vh = glm::normalize(glm::vec3(roughness * ve.x, roughness * ve.y, ve.z));

		float lensq = vh.x * vh.x + vh.y * vh.y;
		glm::vec3 T1 = lensq > 0.0f ? glm::vec3(-vh.y, vh.x, 0.0f) / glm::sqrt(lensq) : glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 T2 = glm::cross(vh, T1);

		float r = glm::sqrt(u1);
		float phi = 2.0f * glm::pi<float>() * u2;
		float t1 = r * glm::cos(phi);
		float t2 = r * glm::sin(phi);
		float s = 0.5f * (1.0f + vh.z);
		t2 = glm::mix(glm::sqrt(1.0f - t1 * t1), t2, s);

		glm::vec3 nh = t1 * T1 + t2 * T2 + glm::sqrt(glm::max(0.0f, 1.0f - t1 * t1 - t2 * t2)) * vh;
		glm::vec3 hLocal = glm::normalize(glm::vec3(roughness * nh.x, roughness * nh.y, glm::max(0.0f, nh.z)));

		return glm::normalize(TBN * hLocal);
	}
}