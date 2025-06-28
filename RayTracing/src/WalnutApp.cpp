#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "renderer.h"
#include "camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(60.0f, 0.1f, 100.0f) {}

	virtual void OnUpdate(float ts)
	{
		m_Camera.on_update(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("FPS: %.1f", 1000.0f / m_LastRenderTime);

		if (ImGui::Button("Render"))
		{
			Render();
		}

		// move light
		if (ImGui::CollapsingHeader("Light Source Controls"))
		{
			ImGui::SliderFloat("x", &m_Renderer.get_light_position().x, -10, 10);
			ImGui::SliderFloat("y", &m_Renderer.get_light_position().y, -10, 10);
			ImGui::SliderFloat("z", &m_Renderer.get_light_position().z, -10, 10);
		}
		ImGui::End();

		ImGui::Begin("Scene");

		if (ImGui::Button("New Sphere")) {
			sphere sphere;
			m_Scene.spheres.push_back(sphere);
		}

		for (size_t i = 0; i < m_Scene.spheres.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::DragFloat3("Centre", glm::value_ptr(m_Scene.spheres[i].centre), 0.1f);
			ImGui::DragFloat("Radius", &m_Scene.spheres[i].radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.spheres[i].albedo));

			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.get_final_image();
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() 
	{
		Timer timer; 

		m_Renderer.on_resize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.on_resize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	renderer m_Renderer;
	camera m_Camera;
	scene m_Scene;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}