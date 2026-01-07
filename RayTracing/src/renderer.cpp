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
		std::for_each(std::execution::par, rows.begin(), rows.end(), [this, y](uint32_t x){ renderImage(x, y); });
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

hit_info renderer::closestHit(const ray& ray, int objectIndex, float hitDistance)
{
	hit_info hitInfo;
	hitInfo.hitDistance = hitDistance;
	hitInfo.objectIndex = objectIndex;


	const object* closestObject = m_activeScene->objects[objectIndex].get();

	hitInfo.worldPosition = ray.origin + hitDistance * ray.direction;
	hitInfo.worldNormal = closestObject->getNormalAt(hitInfo.worldPosition);

	return hitInfo;
}

hit_info renderer::miss(const ray& ray)
{
	hit_info hitInfo;
	hitInfo.hitDistance = -1;
	return hitInfo;
}

glm::vec4 renderer::perPixel(uint32_t x, uint32_t y)
{
	ray current_ray{ m_activeCamera->get_position(), normalize(m_activeCamera->get_ray_direction(x, y))};

	int ray_depth = 5;
	glm::vec3 final_albedo{ 0.0f };
	glm::vec3 attenuation{ 1.0f };

	for (int i = 0; i < ray_depth; i++)
	{
		hit_info hit_info = traceRay(current_ray);

		if (hit_info.hitDistance < 0.0f)
		{
			if (m_settings.skybox)
			{
				float t = 0.5f * current_ray.direction.y + 1.0f;
				glm::vec3 sky_colour = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.6f, 0.7f, 0.9f);
				final_albedo += attenuation * sky_colour;
			}
			else
			{
				final_albedo += attenuation * m_activeScene->background_colour;
			}

			break;
		}

		glm::vec3 local_attenuation{ 1.0f };
		ray scattered_ray;

		const object* object = m_activeScene->objects[hit_info.objectIndex].get();
		material* material = m_activeScene->materials[object->material_index].get();

		if (dynamic_cast<diffuse*>(material))
		{
			for (int i = 0; i < m_activeScene->lights.size(); i++)
			{
				light* light = m_activeScene->lights[i].get();
				glm::vec3 light_direction = light->get_direction(hit_info.worldPosition);

				ray shadow_ray{ hit_info.worldPosition, light_direction };

				if (traceRay(shadow_ray).hitDistance < 0.0f)
				{
					final_albedo += material->albedo / glm::pi<float>()
						* light->get_intensity(hit_info.worldPosition)
						* std::max(0.0f, dot(hit_info.worldNormal, light_direction));
				}
			}
		}

		if (material->scatter(current_ray, scattered_ray, hit_info, local_attenuation))
		{
			attenuation *= local_attenuation;
			current_ray = scattered_ray;
		}
		else 
		{
			break;
		}
	}

	return { final_albedo, 1.0f };
}

void renderer::renderImage(uint32_t x, uint32_t y)
{
	glm::vec4 colour = perPixel(x, y);

	size_t index = y * m_finalImage->GetWidth() + x;

	m_accumulationData[index] += colour;
	glm::vec4 accumulatedColour = m_accumulationData[index];
	accumulatedColour /= m_frameIndex;

	accumulatedColour = clamp(accumulatedColour, glm::vec4(0.0f), glm::vec4(1.0f));
	m_imageData[index] = utils::convertToRGBA(accumulatedColour);
}

hit_info renderer::traceRay(const ray& ray)
{

	if (m_activeScene->objects.size() == 0)
	{
		return miss(ray);
	}

	int closest_object_index = -1;
	float closest_t = FLT_MAX;
	const float T_MIN = 0.001f; // to avoid self-intersection

	std::priority_queue<std::pair<float, BVHNode*>,
						std::vector<std::pair<float, BVHNode*>>,
						std::greater<>> candidate_nodes{};

	BVHNode* root = m_activeScene->bvh.get()->root.get();
	float root_t = root->bounds.hit(ray);

	if (root_t >= 0.0f)
	{
		candidate_nodes.push({ root_t, root });
	}

	while (!candidate_nodes.empty())
	{
		auto [current_node_t, current_node] = candidate_nodes.top();
		candidate_nodes.pop();

		if (current_node_t < closest_t)
		{
			if (current_node->is_leaf())
			{
				for (int index : current_node->object_indices)
				{
					float t = m_activeScene->objects[index]->hit(ray);
					
					if (t >= T_MIN && t < closest_t)
					{
						closest_t = t;
						closest_object_index = index;
					}
				}
			}
			else
			{
				for (auto& child : current_node->children)
				{
					float t = child.get()->bounds.hit(ray);

					if (t >= 0.0f)
					{
						candidate_nodes.push({t, child.get()});
					}
				}
			}
		}
		else 
		{
			break;
		}
	}

	if (closest_object_index < 0)
	{
		return miss(ray);
	}

	return closestHit(ray, closest_object_index, closest_t);
}
