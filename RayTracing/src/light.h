#pragma once
#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class light 
{
public:
	glm::vec3 colour{ 1.0f };
	float intensity = 1.0f;

	virtual glm::vec3 get_direction(const glm::vec3& world_position) const = 0;

	virtual glm::vec3 get_intensity(const glm::vec3& world_position) const = 0;

	virtual glm::vec3& get_vector() = 0;

	virtual ~light() {}
};

class spherical_light : public light 
{
public:
	glm::vec3 position{ 1.0f };

	glm::vec3 get_direction(const glm::vec3& world_position) const override
	{
		return glm::normalize(position - world_position);
	}

	glm::vec3 get_intensity(const glm::vec3& world_position) const
	{
		return colour * intensity / (4.0f * glm::pi<float>() * glm::length(position - world_position));
	}

	glm::vec3& get_vector() override { return position; }
};

class distant_light : public light
{
public:
	glm::vec3 direction{ -1.0f };

	glm::vec3 get_direction(const glm::vec3& world_position) const override
	{
		return -direction;
	}

	glm::vec3 get_intensity(const glm::vec3& world_position) const
	{
		return intensity * colour;
	}


	glm::vec3& get_vector() override { return direction; }
};