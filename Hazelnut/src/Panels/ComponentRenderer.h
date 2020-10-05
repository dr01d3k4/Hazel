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

	template <typename T>
	const std::vector<std::string>& getStringsForEnum() = delete;

	template <>
	const std::vector<std::string>& getStringsForEnum<SceneCamera::ProjectionType>()
	{
		static std::vector<std::string> strings = { "Perspective", "Orthographic" };
		return strings;
	}

	// Note: this expects your enum to be castable to incrementing integers starting from 0 with no holes
	template <typename T>
	void renderEnumSelector(std::string name, T value, std::function<void(T newValue)> changedCallback)
	{
		const std::vector<std::string>& strings = getStringsForEnum<T>();
		const std::string& current = strings[int(value)];

		if (ImGui::BeginCombo(name.c_str(), current.c_str()))
		{
			for (int i = 0; i < strings.size(); ++i)
			{
				bool isSelected = int(value) == i;
				if (ImGui::Selectable(strings[i].c_str(), isSelected))
				{
					changedCallback((T) i);
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}

	class CameraComponentRenderer : public ComponentRenderer<CameraComponent>
	{
	public:
		CameraComponentRenderer() : ComponentRenderer<CameraComponent>("Camera") {}

	private:
		virtual void Draw(Entity entity, CameraComponent& component) override
		{
			auto& camera = component.Camera;

			ImGui::Checkbox("Primary", &component.Primary);

			renderEnumSelector<SceneCamera::ProjectionType>("Projection", camera.GetProjectionType(), [&camera](SceneCamera::ProjectionType v)
			{
				camera.SetProjectionType(v);
			});

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
