#pragma once

#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class IComponentRenderer
	{
	public:
		IComponentRenderer() {}
		virtual ~IComponentRenderer() {}

		virtual void MaybeDraw(Entity entity) = 0;
	};

	template <typename T>
	class ComponentRenderer : public IComponentRenderer
	{
	public:
		ComponentRenderer(std::string name) : m_Name(name) {}

		void MaybeDraw(Entity entity) override
		{
			if (!entity.HasComponent<T>())
			{
				return;
			}

			const size_t componentId = typeid(T).hash_code();
			const char* componentName = m_Name.c_str();
			if (ImGui::TreeNodeEx((void*)componentId, ImGuiTreeNodeFlags_DefaultOpen, componentName))
			{
				Draw(entity, entity.GetComponent<T>());
				ImGui::TreePop();
			}
		}

	private:
		std::string m_Name;

		virtual void Draw(Entity entity, T& component) = 0;
	};

	class TagComponentRenderer : public ComponentRenderer<TagComponent>
	{
	public:
		TagComponentRenderer() : ComponentRenderer<TagComponent>("Tag") {}

	private:
		virtual void Draw(Entity entity, TagComponent& component) override
		{
			auto& tag = component.Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
	};

	class TransformComponentRenderer : public ComponentRenderer<TransformComponent>
	{
	public:
		TransformComponentRenderer() : ComponentRenderer<TransformComponent>("Transform") {}

	private:
		virtual void Draw(Entity entity, TransformComponent& component) override
		{
			auto& transform = component.Transform;
			ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);
		}
	};

	class CameraComponentRenderer : public ComponentRenderer<CameraComponent>
	{
	public:
		CameraComponentRenderer() : ComponentRenderer<CameraComponent>("Camera") {}

	private:
		virtual void Draw(Entity entity, CameraComponent& component) override
		{
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
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
				if (ImGui::DragFloat("Vertical FOV", &verticalFov))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));

				float orthoNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetPerspectiveNearClip(orthoNear);

				float orthoFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetPerspectiveFarClip(orthoFar);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near", &orthoNear))
					camera.SetOrthographicNearClip(orthoNear);

				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far", &orthoFar))
					camera.SetOrthographicFarClip(orthoFar);

				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
			}
		}
	};
}
