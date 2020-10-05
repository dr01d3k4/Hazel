#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Scene/Components.h"

namespace Hazel {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID , m_Context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		/*
			To use ImGui::BeginColumns() and ImGui::EndColumns(), move these from imgui_internal.h to imgui.h:
			`typedef int ImGuiColumnsFlags;      // -> enum ImGuiColumnsFlags_       // Flags: BeginColumns()`

			`IMGUI_API void          BeginColumns(const char* str_id, int count, ImGuiColumnsFlags flags = 0); // setup number of columns. use an identifier to distinguish multiple column sets. close with EndColumns().`
			`IMGUI_API void          EndColumns();                                                             // close columns`

			```
			enum ImGuiColumnsFlags_
			{
				// Default: 0
				ImGuiColumnsFlags_None = 0,
				ImGuiColumnsFlags_NoBorder = 1 << 0,   // Disable column dividers
				ImGuiColumnsFlags_NoResize = 1 << 1,   // Disable resizing columns when clicking on the dividers
				ImGuiColumnsFlags_NoPreserveWidths = 1 << 2,   // Disable column width preservation when adjusting columns
				ImGuiColumnsFlags_NoForceWithinWindow = 1 << 3,   // Disable forcing columns to fit within window
				ImGuiColumnsFlags_GrowParentContentsSize = 1 << 4    // (WIP) Restore pre-1.51 behavior of extending the parent window contents size but _without affecting the columns width at all_. Will eventually remove.
			};
			```
		*/

		if (entity.HasComponent<TagComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TagComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Tag"))
			{
				ImGui::BeginColumns("Tag", 2, 0);

				auto& tag = entity.GetComponent<TagComponent>().Tag;

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, sizeof(buffer), tag.c_str());
				ImGui::Text("Tag");
				ImGui::NextColumn();
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}

				ImGui::EndColumns();
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				ImGui::BeginColumns("Transform", 2, 0);

				auto& transform = entity.GetComponent<TransformComponent>().Transform;
				ImGui::Text("Position");
				ImGui::NextColumn();
				ImGui::DragFloat3("##Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::EndColumns();
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				ImGui::BeginColumns("Camera", 2, 0);
				auto& cameraComponent = entity.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.Camera;

				ImGui::Text("Primary");
				ImGui::NextColumn();
				ImGui::Checkbox("##Primary", &cameraComponent.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				ImGui::NextColumn();
				ImGui::Text("Projection");
				ImGui::NextColumn();
				if (ImGui::BeginCombo("##Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					ImGui::NextColumn();
					ImGui::Text("Vertical FOV");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Vertical FOV", &verticalFov))
						camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

					float orthoNear = camera.GetPerspectiveNearClip();
					ImGui::NextColumn();
					ImGui::Text("Near");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Near", &orthoNear))
						camera.SetPerspectiveNearClip(orthoNear);

					float orthoFar = camera.GetPerspectiveFarClip();
					ImGui::NextColumn();
					ImGui::Text("Far");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Far", &orthoFar))
						camera.SetPerspectiveFarClip(orthoFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					ImGui::NextColumn();
					ImGui::Text("Size");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Size", &orthoSize))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					ImGui::NextColumn();
					ImGui::Text("Near");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Near", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					ImGui::NextColumn();
					ImGui::Text("Far");
					ImGui::NextColumn();
					if (ImGui::DragFloat("##Far", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::NextColumn();
					ImGui::Text("Fixed Aspect Ratio");
					ImGui::NextColumn();
					ImGui::Checkbox("##Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);
				}

				ImGui::EndColumns();
				ImGui::TreePop();
			}
		}
	}

}
