#include "extent.h"

float extent::hit(const ray& ray) const
{
	std::array<float, 7> S;
	std::array<float, 7> T;

	// precompute dot products
	for (int i = 0; i < 7; i++)
	{
		if (active[i])
		{
			S[i] = glm::dot(plane_set_normals[i], ray.origin);
			float denominator = glm::dot(plane_set_normals[i], ray.direction);
			T[i] = glm::abs(denominator) < FLT_EPSILON ? glm::sqrt(FLT_MAX) : 1.0f / denominator;
		}
	}

	float t_near = 0;
	float t_far = FLT_MAX;

	for (int i = 0; i < 7; i++)
	{
		if (active[i])
		{
			float new_t_near = (slabs[i].d_near - S[i]) * T[i];
			float new_t_far = (slabs[i].d_far - S[i]) * T[i];

			if (T[i] < 0)
			{
				std::swap(new_t_near, new_t_far);
			}

			t_near = new_t_near > t_near ? new_t_near : t_near ;
			t_far = new_t_far < t_far ? new_t_far : t_far;

			if (t_near > t_far)
			{
				return -1;
			}
		}
	}

	return t_near;
}

void extent::expand(const extent& other)
{
	for (size_t i = 0; i < 7; i++)
	{
		if (other.active[i])
		{
			if (!active[i])
			{
				slabs[i] = other.slabs[i];
			}
			else
			{
				slabs[i].d_near = std::min(slabs[i].d_near, other.slabs[i].d_near);
				slabs[i].d_far = std::max(slabs[i].d_far, other.slabs[i].d_far);
			}
		}
	}
}
