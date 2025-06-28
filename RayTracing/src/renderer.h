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

	glm::vec4 trace_ray(const scene& scene, const ray& ray);
};