#include "renderer.h"
#include "Walnut/Random.h"
#include "math.h"

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
	ray ray;
	ray.origin = camera.get_position();

	// render every pixel

	for (uint32_t y = 0; y < m_final_image_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image_->GetWidth(); x++)
		{
			ray.direction = camera.get_ray_direction()[x + y * m_final_image_->GetWidth()];


			glm::vec4 color = trace_ray(scene, ray);
			color = clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_image_data_[y * m_final_image_->GetWidth() + x] = utils::convert_to_RGBA(color);
		}
	}


	m_final_image_->SetData(m_image_data_);
}

glm::vec4 renderer::trace_ray(const scene& scene, const ray& ray)
{
	/*
	|| UV Gradient ||

	uint8_t red = (uint8_t)(coord.x * 255.0f);
	uint8_t green = (uint8_t)(coord.y * 255.0f);
	// uint8_t blue = (uint8_t)(((coord.y + coord.x))/2 * 255.0f);

	return 0xff000000 | (green << 8) | red;
	*/

	// ray_direction = glm::normalize(ray_direction); // more expensive

	if (scene.spheres.size() == 0)
	{
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	const sphere* closest_sphere = nullptr;
	float hit_distance = FLT_MAX;

	for (const sphere& sphere : scene.spheres)
	{
		// (B.B)t^2 + 2(B.(A-C))t + ((A-C).(A-C) - r^2) = 0
		// A = ray origin vector (camera)
		// B = ray direction vector
		// t = parameter
		// C = centre of sphere
		// r = radius

		float a = dot(ray.direction, ray.direction);
		float b = 2.0f * dot(ray.direction, ray.origin - sphere.centre);
		float c = dot(ray.origin - sphere.centre, ray.origin - sphere.centre) - sphere.radius * sphere.radius;

		if (float discriminant = b * b - 4.0f * a * c; discriminant >= 0)
		{
			float q = -0.5f * (b > 0) ? (b + sqrt(discriminant)) : -0.5f * (b - sqrt(discriminant)); // ensures calculation is numerically stable

			float t = std::min(q / a, c / q); // t is the entry point
			t = std::max(t, 0.0f); 

			if (t < hit_distance)
			{
				hit_distance = t;
				closest_sphere = &sphere;
			}
		}
	}

	if (closest_sphere != nullptr)
	{
		glm::vec3 hit_point = ray.origin + hit_distance * ray.direction;
		glm::vec3 hit_point_normal = normalize(hit_point - closest_sphere->centre);

		glm::vec3 light_direction = normalize(scene.light_position - hit_point);
		float light_intensity = glm::max(dot(hit_point_normal, light_direction), 0.0f);

		return { closest_sphere->albedo * light_intensity, 1.0f };
	}

	return { 0.0f, 0.0f, 0.0f, 1.0f };
}
