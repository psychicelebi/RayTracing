#pragma once
#include "ray.h"
#include "extent.h"

class object
{
public:
	glm::vec3 position{ 0.0f, 0.0f, -2.0f };

	int material_index = 0;

	virtual float hit(const ray &ray) const = 0;

	virtual extent get_extent(const std::vector<int>& normal_indices) const = 0;

	virtual glm::vec3 getNormalAt(const glm::vec3& worldPosition) const = 0;
};

class sphere : public object
{
public:
	float radius = 0.5f;

	float hit(const ray& ray) const override;

	extent get_extent(const std::vector<int>& normal_indices) const override;

	virtual glm::vec3 getNormalAt(const glm::vec3& worldPosition) const override;
	
};