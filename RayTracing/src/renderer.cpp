#include "renderer.h"

namespace utils
{
	static uint32_t convert_to_RGBA(const glm::vec4& color)
	{
		auto r = (uint8_t)(color.r * 255.0f);
		auto g = (uint8_t)(color.g * 255.0f);
		auto b = (uint8_t)(color.b * 255.0f);
		auto a = (uint8_t)(color.a * 255.0f);

		uint32_t result = a << 24 | b << 16 | g << 8 | r;
		return result;
	}
}

void renderer::on_resize(uint32_t width, uint32_t height)
{
	if (m_final_image_)
	{
		// no resize necessary
		if (m_final_image_->GetWidth() == width && m_final_image_->GetHeight() == height)
		{
			return;
		}

		m_final_image_->Resize(width, height);
	}
	else 
	{
		m_final_image_ = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_image_data_;
	m_image_data_ = new uint32_t[width * height];

	delete[] m_accumulation_data_;
	m_accumulation_data_ = new glm::vec4[width * height];

	m_ImageHorizontalIt.resize(width);
	m_ImageVerticalIt.resize(height);

	for (uint32_t i = 0; i < width; i++)
	{
		m_ImageHorizontalIt[i] = i;
	}

	for (uint32_t i = 0; i < height; i++)
	{
		m_ImageVerticalIt[i] = i;
	}
}

void renderer::render(const scene& scene, const camera& camera)
{
	m_active_camera_ = &camera;
	m_active_scene_ = &scene;

	if (m_frame_index_ == 1)
	{
		memset(m_accumulation_data_, 0, m_final_image_->GetWidth() * m_final_image_->GetHeight() * sizeof(glm::vec4));
	}

	// render every pixel

#define MT 1 // multi-threading
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIt.begin(), m_ImageVerticalIt.end(), [this](uint32_t y) 
		{
			std::for_each(std::execution::par, m_ImageHorizontalIt.begin(), m_ImageHorizontalIt.end(), [this, y](uint32_t x)
				{
					glm::vec4 color = per_pixel(x, y);
					num_primary_rays++;

					m_accumulation_data_[y * m_final_image_->GetWidth() + x] += color;
					glm::vec4 accumulated_colour = m_accumulation_data_[y * m_final_image_->GetWidth() + x];
					accumulated_colour /= m_frame_index_;

					accumulated_colour = clamp(accumulated_colour, glm::vec4(0.0f), glm::vec4(1.0f));
					m_image_data_[y * m_final_image_->GetWidth() + x] = utils::convert_to_RGBA(accumulated_colour);

				});
		});
#else
	for (uint32_t y = 0; y < m_final_image_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image_->GetWidth(); x++)
		{
			glm::vec4 color = per_pixel(x, y);

			m_accumulation_data_[y * m_final_image_->GetWidth() + x] += color;
			glm::vec4 accumulated_colour = m_accumulation_data_[y * m_final_image_->GetWidth() + x];
			accumulated_colour /= m_frame_index_;

			accumulated_colour = clamp(accumulated_colour, glm::vec4(0.0f), glm::vec4(1.0f));
			m_image_data_[y * m_final_image_->GetWidth() + x] = utils::convert_to_RGBA(accumulated_colour);
		}
	}
#endif


	m_final_image_->SetData(m_image_data_);

	if (m_settings_.accumulate)
	{
		m_frame_index_++;
	}
	else 
	{
		m_frame_index_ = 1;
	}
}

hit_info renderer::closest_hit(const ray& ray, int object_index, float hit_distance)
{
	hit_info hit_info;
	hit_info.hit_distance = hit_distance;
	hit_info.object_index = object_index;


	const sphere& closest_sphere = m_active_scene_->spheres[object_index];

	hit_info.world_position = ray.origin + hit_distance * ray.direction;
	hit_info.world_normal = normalize(hit_info.world_position - closest_sphere.centre);

	return hit_info;
}

hit_info renderer::miss(const ray& ray)
{
	hit_info hit_info;
	hit_info.hit_distance = -1;
	return hit_info;
}

glm::vec4 renderer::per_pixel(uint32_t x, uint32_t y)
{
	ray current_ray{ m_active_camera_->get_position(), normalize(m_active_camera_->get_ray_direction()[x + y * m_final_image_->GetWidth()]) };

	int ray_depth = 5;
	glm::vec3 final_albedo{ 0.0f };
	glm::vec3 attenuation{ 1.0f };

	for (int i = 0; i < ray_depth; i++)
	{
		hit_info hit_info = trace_ray(current_ray);
		num_ray_sphere_tests++;

		if (hit_info.hit_distance < 0.0f)
		{
			if (m_settings_.skybox)
			{
				float t = 0.5f * current_ray.direction.y + 1.0f;
				glm::vec3 sky_colour = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.6f, 0.7f, 0.9f);
				final_albedo += attenuation * sky_colour;
			}
			else
			{
				final_albedo += attenuation * m_active_scene_->background_colour;
			}

			break;
		}

		num_ray_sphere_isect++;

		glm::vec3 local_attenuation{ 1.0f };
		ray scattered_ray;

		const sphere& sphere = m_active_scene_->spheres[hit_info.object_index];
		material* material = m_active_scene_->materials[sphere.material_index].get();

		if (dynamic_cast<diffuse*>(material))
		{
			for (int i = 0; i < m_active_scene_->lights.size(); i++)
			{
				light* light = m_active_scene_->lights[i].get();
				glm::vec3 light_direction = light->get_direction(hit_info.world_position);

				ray shadow_ray{ hit_info.world_position + 0.001f * hit_info.world_normal, light_direction };

				if (trace_ray(shadow_ray).hit_distance < 0.0f)
				{
					final_albedo += material->albedo / glm::pi<float>()
						* light->get_intensity(hit_info.world_position)
						* std::max(0.0f, dot(hit_info.world_normal, light_direction));
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

hit_info renderer::trace_ray(const ray& ray)
{

	// ray_direction = glm::normalize(ray_direction); // more expensive

	if (m_active_scene_->spheres.size() == 0)
	{
		return miss(ray);
	}

	int closest_sphere = -1;
	float hit_distance = FLT_MAX;

	for (size_t i = 0; i < m_active_scene_->spheres.size(); i++)
	{
		const sphere& sphere = m_active_scene_->spheres[i];
		float a = dot(ray.direction, ray.direction);
		float b = 2.0f * dot(ray.direction, ray.origin - sphere.centre);
		float c = dot(ray.origin - sphere.centre, ray.origin - sphere.centre) - sphere.radius * sphere.radius;

		if (float discriminant = b * b - 4.0f * a * c; discriminant >= 0)
		{
			float q = (b > 0) ? -0.5f * (b + sqrt(discriminant)) : -0.5f * (b - sqrt(discriminant)); // ensures calculation is numerically stable

			float t = std::min(q / a, c / q); // t is the entry point

			if (t > 0 && t < hit_distance)
			{
				hit_distance = t;
				closest_sphere = (int)i;
			}
		}
	}

	if (closest_sphere < 0)
	{
		return miss(ray);
	}

	return closest_hit(ray, closest_sphere, hit_distance);
}
