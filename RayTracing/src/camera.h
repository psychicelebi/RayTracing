#pragma once

#include <glm/glm.hpp>
#include <vector>

class camera
{
public:
	camera(float verticalFOV, float nearClip, float farClip);

	bool on_update(float ts);
	void on_resize(uint32_t width, uint32_t height);

	const glm::mat4& get_projection() const { return m_Projection; }
	const glm::mat4& get_inverse_projection() const { return m_InverseProjection; }
	const glm::mat4& get_view() const { return m_View; }
	const glm::mat4& get_inverse_view() const { return m_InverseView; }
	
	const glm::vec3& get_position() const { return m_Position; }
	const glm::vec3& get_direction() const { return m_ForwardDirection; }

	const std::vector<glm::vec3>& get_ray_direction() const { return m_RayDirections; }

	float get_rotation_speed();
private:
	void recalculate_projection();
	void recalculate_view();
	void recalculate_ray_directions();
private:
	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;

	glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
	glm::vec3 m_ForwardDirection{0.0f, 0.0f, 0.0f};

	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};
