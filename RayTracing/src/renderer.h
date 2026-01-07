#pragma once

#include "Walnut/Image.h"

#include "camera.h"
#include "ray.h"
#include "scene.h"
#include "material.h"
#include "BVH.h"
#include "hit_info.h"
#include "Walnut/Random.h"

#include <memory>
#include <execution>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <atomic>
#include <ranges>

class renderer
{
public:
	void onResize(uint32_t width, uint32_t height);
	void render(const scene& scene, const camera& camera);
	void resetFrameIndex() { m_frameIndex = 1; }

	std::shared_ptr<Walnut::Image> getFinalImage() const { return m_finalImage; }

	struct settings
	{
		bool accumulate{ false };
		bool skybox{ false };
		int rayDepth{ 5 };
	};

	settings& getSettings() { return m_settings; }

private:
	std::shared_ptr<Walnut::Image> m_finalImage;
	std::vector<uint32_t> m_imageData{};
	std::vector<glm::vec4> m_accumulationData{};
	uint32_t m_frameIndex{ 1 };

	settings m_settings;

	const scene* m_activeScene{};
	const camera* m_activeCamera{};

	void renderPixel(uint32_t x, uint32_t y);

	glm::vec4 shadePixel(uint32_t x, uint32_t y); // RayGen in DX and Vulkan
};