#pragma once
#include "core/thread/taskScheduler.h"
#include "riow/scene.h"
#include "riow/camera.h"
#include "riow/color.h"

//#Todo: motion blur.
//#Todo: BVH.
//#Todo: Volumetric.

namespace dxray::riow
{
	/// <summary>
	/// Render pipeline configuration.
	/// </summary>
	struct RendererPipeline final
	{
		u8 MaxTraceDepth = 8;
		u8 SuperSampleFactor = 2;
		u8 DepthOfFieldSampleCount = 4;
		u8 ClusterSize = 4;
	};

	/// <summary>
	/// The renderer is responsible for the construction and dispatching of rays.
	/// </summary>
	class Renderer final
	{
	public:
		Renderer();
		~Renderer() = default;

		void SetRenderPipeline(const RendererPipeline& a_pipeline);
		void SetCamera(const Camera& a_camera);
		void SetBackgroundColor(const Color& a_color);

		void Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer);

	private:
		Color TraceRayColor(const riow::Ray& a_ray, const riow::Scene& a_scene, const u8 a_maxTraceDepth) const;

		Camera m_camera;
		RendererPipeline m_pipelineConfiguration;
		TaskScheduler m_taskScheduler;
		Color m_backgroundColor;
	};

	inline void Renderer::SetRenderPipeline(const RendererPipeline& a_pipeline)
	{
		m_pipelineConfiguration = a_pipeline;
	}

	inline void Renderer::SetCamera(const Camera& a_camera)
	{
		m_camera = a_camera;
	}

    inline void Renderer::SetBackgroundColor(const Color& a_color)
    {
		m_backgroundColor = a_color;
    }
}