#pragma once

#include "Walnut/Image.h"

#include "camera.h"
#include "ray.h"
#include "scene.h"

#include <memory>
#include <glm/glm.hpp>

class renderer
{
public:
	renderer() = default;

	void on_resize(uint32_t width, uint32_t height);
	void render(const scene& scence, const camera& camera);

	std::shared_ptr<Walnut::Image> get_final_image() const
	{
		return m_final_image_;
	}

private:
	std::shared_ptr<Walnut::Image> m_final_image_;
	uint32_t* m_image_data_ = nullptr;
	const scene* m_active_scene_ = nullptr;
	const camera* m_active_camera_ = nullptr;

	struct hit_info
	{
		float hit_distance;
		glm::vec3 world_position;
		glm::vec3 world_normal;

		int object_index;
	};

	hit_info trace_ray(const ray& ray);
	hit_info closest_hit(const ray& ray, int object_index, float hit_distance);
	hit_info miss(const ray& ray);

	glm::vec4 per_pixel(uint32_t x, uint32_t y); // RayGen in DX and Vulkan
};