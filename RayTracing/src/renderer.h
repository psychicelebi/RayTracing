#pragma once

#include "Walnut/Image.h"

#include "camera.h"
#include "ray.h"
#include "scene.h"
#include "material.h"
#include "hit_info.h"
#include "Walnut/Random.h"

#include <memory>
#include <execution>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <atomic>

class renderer
{
public:
	renderer() = default;

	void on_resize(uint32_t width, uint32_t height);
	void render(const scene& scence, const camera& camera);
	void reset_frame_index() { m_frame_index_ = 1; }

	std::shared_ptr<Walnut::Image> get_final_image() const
	{
		return m_final_image_;
	}

	struct settings
	{
		bool accumulate = true;
		bool skybox = false;
	};
	settings& get_settings() { return m_settings_; }

	inline static std::atomic<uint64_t> num_ray_sphere_tests{ 0 };
	inline static std::atomic<uint64_t> num_ray_sphere_isect{ 0 };
	inline static std::atomic<uint64_t> num_primary_rays{ 0 };

private:
	std::shared_ptr<Walnut::Image> m_final_image_;
	uint32_t* m_image_data_ = nullptr;
	glm::vec4* m_accumulation_data_ = nullptr;
	uint32_t m_frame_index_ = 1;

	settings m_settings_;

	std::vector<uint32_t> m_ImageHorizontalIt, m_ImageVerticalIt;

	const scene* m_active_scene_ = nullptr;
	const camera* m_active_camera_ = nullptr;

	hit_info trace_ray(const ray& ray);
	hit_info closest_hit(const ray& ray, int object_index, float hit_distance);
	hit_info miss(const ray& ray);

	glm::vec4 per_pixel(uint32_t x, uint32_t y); // RayGen in DX and Vulkan
};