#include "material.h"
#include "BRDF.h"

// Diffuse implementation
bool diffuse::scatter(const ray& currentRay, ray& scatteredRay, const hit_info& hitInfo, glm::vec3& attenuation) const
{
	glm::vec3 scatterDirection = hitInfo.worldNormal + Walnut::Random::InUnitSphere();

	if (glm::length(scatterDirection) < 0.001f )
	{
		scatterDirection = hitInfo.worldNormal;
	}

	scatteredRay.origin = hitInfo.worldPosition;
	scatteredRay.direction = glm::normalize(scatterDirection);

	attenuation = baseColour;
	return true;
}

glm::vec3 diffuse::brdf(const glm::vec3& rayDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	return baseColour / glm::pi<float>();
}

// Metal implementation
bool metal::scatter(const ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const
{
	glm::vec3 reflection = reflect(current_ray.direction, hit_info.worldNormal);
	scattered_ray.origin = hit_info.worldPosition;
	scattered_ray.direction = normalize(reflection + roughness * Walnut::Random::Vec3(-1.0f, 1.0f));

	attenuation = baseColour;
	return true;
}

glm::vec3 metal::brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	glm::vec3 halfVector = glm::normalize(viewDirection + lightDirection);

	float dotNV = glm::clamp(glm::dot(normal, lightDirection), 0.001f, 1.0f); // avoids 0 division
	float dotNL = glm::clamp(glm::dot(normal, lightDirection), 0.001f, 1.0f);
	float dotNH = glm::clamp(glm::dot(normal, halfVector), 0.0f, 1.0f);
	float dotVH = glm::clamp(glm::dot(normal, halfVector), 0.0f, 1.0f);

	float D = BRDF::distributionGGX(dotNH, roughness);
	glm::vec3 F = BRDF::fresnelSchlick(dotVH, baseColour);
	float G = BRDF::geometrySmith(dotNV, dotNL, roughness);

	glm::vec3 numerator = D * F * G;
	float denominator = 4 * dotNL * dotNV;

	return numerator / denominator;
}

// Dielectric implementation
bool dielectric::scatter(const ray& current_ray, ray& scattered_ray, const hit_info& hit_info, glm::vec3& attenuation) const
{
	static std::mt19937 rng{ std::random_device{}() };
	static std::uniform_real_distribution<float> probability(0.0f, 1.0f);

	glm::vec3 normal = hit_info.worldNormal;
	float n1 = 1.0f;
	float n2 = refractive_index;
	float cos_i = glm::min(glm::dot(current_ray.direction, normal), 1.0f); // cosine of incident angle

	if (cos_i > 0.0f)
	{
		normal = -normal;
		std::swap(n1, n2);
	}

	float abs_cos_i = glm::abs(cos_i);
	float sin_i = glm::sqrt(1 - abs_cos_i * abs_cos_i); // sine of incident angle
	float eta = n1 / n2;
	float reflectance = calculate_reflectance(abs_cos_i, n1, n2);

	if (eta * sin_i > 1.0f || reflectance >= Random::getReal<float>(0.0f, 1.0f))
	{
		glm::vec3 reflection = reflect(current_ray.direction, normal);
		scattered_ray.direction = normalize(reflection);
	}
	else
	{
		glm::vec3 refracted_perp = eta * (current_ray.direction + abs_cos_i * normal);
		glm::vec3 refracted_parallel = -glm::sqrt(glm::abs(1.0f - glm::dot(refracted_perp, refracted_perp))) * normal;
		scattered_ray.direction = refracted_perp + refracted_parallel;
	}

	scattered_ray.origin = hit_info.worldPosition;
	attenuation = baseColour;
	return true;
}

glm::vec3 dielectric::brdf(const glm::vec3& viewDirection, const glm::vec3 lightDirection, const glm::vec3& normal) const
{
	return glm::vec3();
}

double dielectric::calculate_reflectance(const float cos_i, const float n1, const float n2) const
{
	float base_reflectance = (n1 - n2) / (n1 + n2);
	base_reflectance *= base_reflectance;
	return base_reflectance + (1.0f - base_reflectance) * glm::pow((1.0f - cos_i), 5);
}