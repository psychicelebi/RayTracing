#include "renderer.h"
#include "Walnut/Random.h"
#include "math.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

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
}

void renderer::render(const scene& scene, const camera& camera)
{
	m_active_camera_ = &camera;
	m_active_scene_ = &scene;

	// render every pixel

	for (uint32_t y = 0; y < m_final_image_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image_->GetWidth(); x++)
		{
			glm::vec4 color = per_pixel(x, y);
			color = clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_image_data_[y * m_final_image_->GetWidth() + x] = utils::convert_to_RGBA(color);
		}
	}


	m_final_image_->SetData(m_image_data_);
}

renderer::hit_info renderer::closest_hit(const ray& ray, int object_index, float hit_distance)
{
	renderer::hit_info hit_info;
	hit_info.hit_distance = hit_distance;
	hit_info.object_index = object_index;


	const sphere& closest_sphere = m_active_scene_->spheres[object_index];

	hit_info.world_position = ray.origin + hit_distance * ray.direction;
	hit_info.world_normal = normalize(hit_info.world_position - closest_sphere.centre);

	return hit_info;
}

renderer::hit_info renderer::miss(const ray& ray)
{
	hit_info hit_info;
	hit_info.hit_distance = -1;
	return hit_info;
}

glm::vec4 renderer::per_pixel(uint32_t x, uint32_t y)
{
	ray ray{ m_active_camera_->get_position(), normalize(m_active_camera_->get_ray_direction()[x + y * m_final_image_->GetWidth()]) };

	hit_info hit_info = trace_ray(ray);

	int max_bounces = 2;
	glm::vec3 final_albedo = { 0.0f, 0.0f, 0.0f };
	glm::vec3 attenuation = { 1.0f, 1.0f, 1.0f };
	
	for (int i = 0; i <= max_bounces; i++)
	{
		if (hit_info.hit_distance < 0.0f)
		{
			glm::vec3 sky_color = { 0.6f, 0.7f, 0.9f };
			final_albedo += attenuation * sky_color;
			break;
		}

		glm::vec3 light_direction = normalize(m_active_scene_->light_position - hit_info.world_position);
		float light_intensity = glm::max(dot(hit_info.world_normal, light_direction), 0.0f);

		const sphere& sphere = m_active_scene_->spheres[hit_info.object_index];
		const material& material = m_active_scene_->materials[sphere.material_index];

		final_albedo += attenuation * material.albedo * light_intensity;
		attenuation *= material.albedo;
		attenuation *= 0.7f;

		ray.origin = hit_info.world_position + hit_info.world_normal * 0.001f;
		ray.direction = reflect(ray.direction, hit_info.world_normal + material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

		hit_info = trace_ray(ray);
	}

	return { final_albedo, 1.0f };
}

renderer::hit_info renderer::trace_ray(const ray& ray)
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
		// (B.B)t^2 + 2(B.(A-C))t + ((A-C).(A-C) - r^2) = 0
		// A = ray origin vector (camera)
		// B = ray direction vector
		// t = parameter
		// C = centre of sphere
		// r = radius

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
