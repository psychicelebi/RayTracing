#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "renderer.h"
#include "camera.h"

#include <glm/gtc/type_ptr.hpp>
#include <limits>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(60.0f, 0.1f, 100.0f) 
	{
		m_Scene.materials.emplace_back(std::make_unique<diffuse>());
	}

	virtual void OnUpdate(float ts)
	{
		if(m_Camera.on_update(ts))
			m_Renderer.resetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		bool changed = false;

		ImGui::Begin("Settings");
		{
			ImGui::Text("Last render: %.3fms", m_LastRenderTime);
			ImGui::Text("FPS: %.1f", 1000.0f / m_LastRenderTime);

			if (ImGui::Button("Render"))
			{
				Render();
			}

			ImGui::Checkbox("Accumulate", &m_Renderer.getSettings().accumulate);

			if (ImGui::Button("Reset"))
			{
				m_Renderer.resetFrameIndex();
			}
		}
		ImGui::End();

		ImGui::Begin("Scene");
		{
			// display materials
			ImGui::Text("Materials");
			ImGui::BeginChild("Materials", ImVec2(0, 200), true);
			{
				for (size_t i = 0; i < m_Scene.materials.size(); i++)
				{
					ImGui::PushID(i);

					std::string header_title = "Material #" + std::to_string(i);
					if (i == 0)
						header_title = "Default Material";

					material* material = m_Scene.materials[i].get();

					if (ImGui::CollapsingHeader(header_title.c_str()))
					{
						ImGui::ColorEdit3("Albedo", glm::value_ptr(material->baseColour));

						if (auto* metal_material = dynamic_cast<metal*>(material))
						{
							ImGui::DragFloat("Roughness", &metal_material->roughness, 0.05f, 0.001f, 1.0f);
						}
						if (auto* emissive_material = dynamic_cast<emissive*>(material))
						{
							ImGui::ColorEdit3("Colour", glm::value_ptr(emissive_material->baseColour));
							ImGui::DragFloat("Intensity", &emissive_material->emissionStrength, 0.05f, 1.0f, 50.0f);
						}

						ImGui::Separator();
					}

					ImGui::PopID();
				}

				if (ImGui::Button("New Material"))
					ImGui::OpenPopup("Select Material");

				if (ImGui::BeginPopup("Select Material"))
				{
					if (ImGui::Button("Metal"))
					{
						m_Scene.materials.emplace_back(std::make_unique<metal>());
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("Diffuse"))
					{
						m_Scene.materials.emplace_back(std::make_unique<diffuse>());
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("Emissive"))
					{
						m_Scene.materials.emplace_back(std::make_unique<emissive>());
						ImGui::CloseCurrentPopup();
					}

					/*
					if (ImGui::Button("Dielectric"))
					{
						m_Scene.materials.emplace_back(std::make_unique<dielectric>());
						ImGui::CloseCurrentPopup();
					}
					*/

					ImGui::EndPopup();
				}
			}
			ImGui::EndChild();

			// display objects
			ImGui::Text("Objects");
			ImGui::BeginChild("Objects", ImVec2(0, 200), true);
			{
				for (size_t i = 0; i < m_Scene.objects.size(); i++)
				{
					ImGui::PushID(i);

					std::string header_title = "Object #" + std::to_string(i + 1);

					if (ImGui::CollapsingHeader(header_title.c_str()))
					{
						object* object = m_Scene.objects[i].get();

						changed |= ImGui::DragFloat3("Position", glm::value_ptr(object->position), 0.05f);
						changed |= ImGui::DragInt("Material", &object->material_index, 1.0f, 0, (int)m_Scene.materials.size() - 1);

						if (auto* sphere_object = dynamic_cast<sphere*>(object))
						{
							changed |= ImGui::DragFloat("Radius", &sphere_object->radius, 0.05f);
						}

						ImGui::Separator();
					}

					ImGui::PopID();
				}

				if (ImGui::Button("New Sphere"))
				{
					m_Scene.objects.emplace_back(std::make_unique<sphere>());
					changed |= true;
				}
			}

			ImGui::EndChild();

			// display background

			ImGui::Text("Background");
			ImGui::BeginChild("Background", ImVec2(0, 200), true);
			{
				ImGui::Checkbox("Enable Skybox", &m_Renderer.getSettings().skybox);

				ImGui::BeginDisabled(m_Renderer.getSettings().skybox);
				{
					ImGui::ColorEdit3("Background Colour", glm::value_ptr(m_Scene.backgroundColour));
				}
				ImGui::EndDisabled();
			}
			ImGui::EndChild();
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		{
			m_ViewportWidth = ImGui::GetContentRegionAvail().x;
			m_ViewportHeight = ImGui::GetContentRegionAvail().y;

			auto image = m_Renderer.getFinalImage();
			if (image)
			{
				ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if (changed)
		{
			m_Scene.bvh = std::make_unique<BVH>(m_Scene.objects);
		}

		Render();
	}

	void Render() 
	{
		Timer timer; 

		m_Renderer.onResize(m_ViewportWidth, m_ViewportHeight);
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