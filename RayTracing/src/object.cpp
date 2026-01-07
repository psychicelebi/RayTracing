#include "object.h"


// Sphere implementation
float sphere::hit(const ray& ray) const
{
	float a = dot(ray.direction, ray.direction);
	float b = 2.0f * dot(ray.direction, ray.origin - position);
	float c = dot(ray.origin - position, ray.origin - position) - radius * radius;

	if (float discriminant = b * b - 4.0f * a * c; discriminant >= 0)
	{
		float q = (b > 0) ? -0.5f * (b + sqrt(discriminant)) : -0.5f * (b - sqrt(discriminant)); // ensures calculation is numerically stable

		return std::min(q / a, c / q); // entry point
	}
	 
	return -1;
}

extent sphere::get_extent(const std::vector<int>& normal_indices) const
{
	extent extent{};
	for (size_t i = 0; i < normal_indices.size(); i++)
	{
		int normal_index = normal_indices[i];
		float d_near = glm::dot(position, extent::plane_set_normals[normal_index]) - radius;
		float d_far = glm::dot(position, extent::plane_set_normals[normal_index]) + radius;

		extent.slabs[normal_index] = { d_near, d_far };
		extent.active.set(normal_index);
	}

	return extent;
}

glm::vec3 sphere::getNormalAt(const glm::vec3& worldPosition) const
{
	return glm::normalize(worldPosition - position);
}
