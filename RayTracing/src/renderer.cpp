#include "renderer.h"

namespace utils
{
	static uint32_t convertToRGBA(const glm::vec4& color)
	{
		auto r = (uint8_t)(color.r * 255.0f);
		auto g = (uint8_t)(color.g * 255.0f);
		auto b = (uint8_t)(color.b * 255.0f);
		auto a = (uint8_t)(color.a * 255.0f);

		uint32_t result = a << 24 | b << 16 | g << 8 | r;
		return result;
	}

	static glm::vec3 getSkyColour(const ray& ray)
	{
		float t = 0.5f * (ray.direction.y + 1.0f);
		glm::vec3 skyColour = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.6f, 0.7f, 0.9f);

		return skyColour;
	}
}

void renderer::onResize(uint32_t width, uint32_t height)
{
	if (m_finalImage)
	{
		// no resize necessary
		if (m_finalImage->GetWidth() == width && m_finalImage->GetHeight() == height)
		{
			return;
		}

		m_finalImage->Resize(width, height);
	}
	else 
	{
		m_finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	m_imageData.resize(width * height);
	m_accumulationData.resize(width * height);
}

void renderer::render(const scene& scene, const camera& camera)
{
	m_activeCamera = &camera;
	m_activeScene = &scene;

	if (m_frameIndex == 1)
	{
		std::fill(m_accumulationData.begin(), m_accumulationData.end(), glm::vec4(0.0f));
	}

	// render every pixel

	auto cols = std::views::iota(0u, m_finalImage->GetHeight());
	auto rows = std::views::iota(0u, m_finalImage->GetWidth());


#define MT 1 // multi-threading
#if MT

	std::for_each(std::execution::par, cols.begin(), cols.end(), [this, rows](uint32_t y)
	{
		std::for_each(std::execution::par, rows.begin(), rows.end(), [this, y](uint32_t x){ renderPixel(x, y); });
	});
#else
	for (auto y : cols)
	{
		for (auto x : rows)
		{
			renderImage(x, y);
		}
	}
#endif

	m_finalImage->SetData(m_imageData.data());

	if (m_settings.accumulate)
	{
		m_frameIndex++;
	}
	else 
	{
		m_frameIndex = 1;
	}
}

glm::vec4 renderer::shadePixel(uint32_t x, uint32_t y)
{
	ray currentRay{ m_activeCamera->getPosition(), normalize(m_activeCamera->getRayDirection(x, y))};

	glm::vec3 radiance{ 0.0f };
	glm::vec3 attenuation{ 1.0f };

	constexpr float SHADOW_BIAS = 0.001f;

	for (int i = 0; i < m_settings.rayDepth; i++)
	{
		hit_info hitInfo = m_activeScene->traceRay(currentRay);

		if (!hitInfo.didHit())
		{
			if (m_settings.skybox)
			{
				radiance += attenuation * utils::getSkyColour(currentRay);
			}
			else
			{
				radiance += attenuation * m_activeScene->backgroundColour;
			}

			break;
		}

		glm::vec3 localAttenuation{ 1.0f };
		ray scatteredRay;

		material* material = m_activeScene->materials[hitInfo.materialIndex].get();

		// TO-DO: implement brdf & area lights
		if (dynamic_cast<diffuse*>(material))
		{
			for (int i = 0; i < m_activeScene->lights.size(); i++)
			{
				light* light = m_activeScene->lights[i].get();
				glm::vec3 lightDirection = light->getDirection(hitInfo.worldPosition);

				ray shadowRay{ hitInfo.worldPosition + hitInfo.worldNormal * SHADOW_BIAS, lightDirection };

				if (!m_activeScene->traceRay(shadowRay).didHit())
				{
					radiance += material->albedo / glm::pi<float>()
						* light->getIntensity(hitInfo.worldPosition)
						* std::max(0.0f, dot(hitInfo.worldNormal, lightDirection))
						* attenuation;
				}
			}
		}

		if (material->scatter(currentRay, scatteredRay, hitInfo, localAttenuation))
		{
			attenuation *= localAttenuation;
			currentRay = scatteredRay;
		}
		else 
		{
			break;
		}
	}

	return { radiance, 1.0f };
}

void renderer::renderPixel(uint32_t x, uint32_t y)
{
	glm::vec4 colour = shadePixel(x, y);

	uint32_t index = y * m_finalImage->GetWidth() + x;

	m_accumulationData[index] += colour;
	glm::vec4 accumulatedColour = m_accumulationData[index];
	accumulatedColour /= m_frameIndex;

	accumulatedColour = clamp(accumulatedColour, glm::vec4(0.0f), glm::vec4(1.0f));
	m_imageData[index] = utils::convertToRGBA(accumulatedColour);
}
