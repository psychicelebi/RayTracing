#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <array>
#include <bitset>

struct slab
{
	float d_near;
	float d_far;
};

class extent
{
public:

	inline static const std::array<glm::vec3, 7> plane_set_normals
	{
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1),
		glm::vec3(glm::sqrt(3) / 3.0f,  glm::sqrt(3) / 3.0f, glm::sqrt(3) / 3.0f),
		glm::vec3(-glm::sqrt(3) / 3.0f,  glm::sqrt(3) / 3.0f, glm::sqrt(3) / 3.0f),
		glm::vec3(-glm::sqrt(3) / 3.0f, -glm::sqrt(3) / 3.0f, glm::sqrt(3) / 3.0f),
		glm::vec3(glm::sqrt(3) / 3.0f, -glm::sqrt(3) / 3.0f, glm::sqrt(3) / 3.0f)
	};

	std::array<slab, 7> slabs{};
	std::bitset<7> active{};

	extent() {}
	
	void expand(const extent& other)
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
	
};