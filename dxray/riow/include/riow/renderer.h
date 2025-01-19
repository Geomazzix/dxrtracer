#pragma once
#include "riow/scene.h"
#include "riow/camera.h"

namespace dxray::riow
{
	/// <summary>
	/// Render pipeline configuration.
	/// </summary>
	struct RendererPipeline
	{
		//#Note: Empty for now, later on used to toggle effects on or off.
	};

	/// <summary>
	/// The renderer is responsible for the construction and dispatching of rays.
	/// </summary>
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;

		void SetRenderPipeline(const RendererPipeline& a_pipeline);
		void SetCamera(const Camera& a_camera);

		void Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer);

	private:
		vath::Vector3 ComputeRayColor(const riow::Ray& a_ray, const riow::Scene& a_scene);
		
		Camera m_camera;
		RendererPipeline m_pipelineConfiguration;
	};

	inline void Renderer::SetRenderPipeline(const RendererPipeline& a_pipeline)
	{
		m_pipelineConfiguration = a_pipeline;
	}

	inline void Renderer::SetCamera(const Camera& a_camera)
	{
		m_camera = a_camera;
	}
}