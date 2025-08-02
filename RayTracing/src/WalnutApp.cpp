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
		: m_Camera(60.0f, 0.1f, 100.0f) 
	{
		material& default_material = m_Scene.materials.emplace_back();
	}

	virtual void OnUpdate(float ts)
	{
		if(m_Camera.on_update(ts))
			m_Renderer.reset_frame_index();
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

		ImGui::Checkbox("Accumulate", &m_Renderer.get_settings().accumulate); 

		if (ImGui::Button("Reset"))
		{
			m_Renderer.reset_frame_index();
		}

		ImGui::End();

		ImGui::Begin("Scene");

		// display materials
		ImGui::Text("Materials");
		ImGui::BeginChild("Materials", ImVec2(0,200), true);

		for (size_t i = 0; i < m_Scene.materials.size(); i++)
		{
			ImGui::PushID(i);

			std::string header_title = "Material #" + std::to_string(i);
			if (i == 0)
				header_title = "Default Material";

			if (ImGui::CollapsingHeader(header_title.c_str()))
			{
				material& material = m_Scene.materials[i];
				ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
				ImGui::DragFloat("Roughness", &material.roughness, 0.05f, 0.0f, 1.0f);
				ImGui::DragFloat("Metallic", &material.metallic, 0.05f, 0.0f, 1.0f);

				ImGui::Separator();
			}

			ImGui::PopID();
		}

		if (ImGui::Button("New Material"))
		{
			material material;
			m_Scene.materials.push_back(material);
		}

		ImGui::EndChild();

		// display spheres
		ImGui::Text("Spheres");
		ImGui::BeginChild("Spheres", ImVec2(0, 200), true);

		for (size_t i = 0; i < m_Scene.spheres.size(); i++)
		{
			ImGui::PushID(i);

			std::string header_title = "Sphere #" + std::to_string(i + 1);

			if (ImGui::CollapsingHeader(header_title.c_str()))
			{
				sphere& sphere = m_Scene.spheres[i];
				ImGui::DragFloat3("Centre", glm::value_ptr(sphere.centre), 0.05f);
				ImGui::DragFloat("Radius", &sphere.radius, 0.05f);
				ImGui::DragInt("Material", &sphere.material_index, 1.0f, 0, (int)m_Scene.materials.size() - 1);

				ImGui::Separator();
			}

			ImGui::PopID();
		}

		if (ImGui::Button("New Sphere"))
		{
			sphere sphere;
			m_Scene.spheres.push_back(sphere);
		}

		ImGui::EndChild();

		// move light
		ImGui::Text("Light Source Controls");
		ImGui::BeginChild("Light Source Controls", ImVec2(0, 200), true);

		ImGui::SliderFloat("x", &m_Scene.light_position.x, -10, 10);
		ImGui::SliderFloat("y", &m_Scene.light_position.y, -10, 10);
		ImGui::SliderFloat("z", &m_Scene.light_position.z, -10, 10);

		ImGui::EndChild();
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