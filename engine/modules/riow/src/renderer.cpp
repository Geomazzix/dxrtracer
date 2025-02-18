#include "riow/renderer.h"
#include "riow/material.h"

namespace dxray::riow
{
	inline vath::Vector2f GetRandom2dUnitDirection()
	{
		const fp32 angle = vath::RandomNumber<fp32>() * 2.0f * vath::Pi<fp32>();
		const vath::Vector2f direction(std::cosf(angle), std::sinf(angle));
		return direction * std::sqrtf(vath::RandomNumber<fp32>());
	}

    Renderer::Renderer() :
		m_taskScheduler(2),
		m_backgroundColor(0.0f)
    { }

	void Renderer::Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer)
	{
		//Ray image plane.
		const vath::Vector3f cameraPosition = m_camera.GetPosition();
		const vath::Vector2u32 viewportDimsInPx = m_camera.GetViewportDimensionsInPx();
		const vath::Rect<fp32> viewportRect = m_camera.GetViewportRect();
		const fp32 cameraShutterSpeed = m_camera.GetShutterSpeed();
		const vath::Vector2f pixelDelta(viewportRect.Width / static_cast<fp32>(viewportDimsInPx.x), viewportRect.Height / static_cast<fp32>(viewportDimsInPx.y));
		const vath::Vector2f pixelCenter = pixelDelta * 0.5f;

		//Anti-aliasing.
		const u8 sampleSize = m_pipelineConfiguration.SuperSampleFactor;
		const u8 sampleCount = sampleSize * sampleSize;
        const fp32 pixelSampleSize = static_cast<fp32>(sampleSize) / sampleCount;
		const fp32 superSampleReciprocal = 1.0f / sampleCount;

		//Depth of field.
		const fp32 focalLength = m_camera.GetFocalLength();
		const fp32 lensRadius = m_camera.GetAperture() / 2.0f;
		const u8 dofSampleCount = m_pipelineConfiguration.DepthOfFieldSampleCount;
		const fp32 dofReciprocal = 1.0f / dofSampleCount;

		//Task threading.
		const vath::Vector2u8 clusterSize(m_pipelineConfiguration.ClusterSize, m_pipelineConfiguration.ClusterSize);

        DXRAY_INFO("=================================");
        DXRAY_INFO("Loaded render pipeline:");
        DXRAY_INFO("Image dimensions: {}, {}", viewportDimsInPx.x, viewportDimsInPx.y);
        DXRAY_INFO("AA sample size {}", sampleSize);
        DXRAY_INFO("DoF sampel count {}", dofSampleCount);
        DXRAY_INFO("=================================");
        DXRAY_INFO("Threading setup:");
        DXRAY_INFO("Num worker threads: {}", m_taskScheduler.GetWorkerCount());
        DXRAY_INFO("Ray Cluster size {}, {}", clusterSize.x, clusterSize.y);
        DXRAY_INFO("=================================");
        DXRAY_INFO("Rendering...");

		//Sample the depth of field of a super-sampled pixel.
		auto SampleDepthOfField = [=](const vath::Vector3f a_rayDirection)
		{
			Color sampleColor(0.0f);
			const vath::Vector3f focalPoint = cameraPosition + a_rayDirection * focalLength;
			for (u32 si = 0; si < dofSampleCount; ++si)
			{
				const vath::Vector2f diskSample = GetRandom2dUnitDirection();
				const vath::Vector3 rayOrigin = cameraPosition + vath::Vector3f(diskSample.x, diskSample.y, 0.0f) * lensRadius;

				//#Note: Shutter speed is randomly sampled so all motion is visible on the image - a real camera needs 1/100 samples to capture a full second of motion,
				//which is way over the speed of what a CPU path tracer can do, games have a target framerate of 1/60 (most often).
				const fp32 shutterSpeed = vath::RandomNumber<fp32>(0.0f, cameraShutterSpeed);
				const riow::Ray camRay(rayOrigin, focalPoint - rayOrigin, shutterSpeed);
				sampleColor += TraceRayColor(camRay, a_scene, m_pipelineConfiguration.MaxTraceDepth);
			}

			return sampleColor * dofReciprocal;
		};

		//Super sample a pixel location.
		auto SuperSamplePixel = [=](const vath::Vector2u32& a_pixelIndex)
		{
            Color pixelColor(0.0f);
            for (u32 sy = 1; sy <= sampleSize; ++sy)
            {
                for (u32 sx = 1; sx <= sampleSize; ++sx)
                {
					//Anti-aliasing.
                    const fp32 r = vath::RandomNumber<fp32>();
                    const vath::Vector2f sampleOffset(
						static_cast<fp32>(sx) / sampleSize - pixelSampleSize * r,
						static_cast<fp32>(sy) / sampleSize - pixelSampleSize * r
					);

					const vath::Vector2f pixelOffset(
						a_pixelIndex.x * pixelDelta.x + sampleOffset.x * pixelDelta.x,
						a_pixelIndex.y * pixelDelta.y + sampleOffset.y * pixelDelta.y
                    );

					const vath::Vector3f rayDirection(m_camera.GetWorldTransform() * vath::Vector4f(
						viewportRect.x + pixelOffset.x,
						viewportRect.y + pixelOffset.y,
						1.0f,
						0.0f
					));

					pixelColor += SampleDepthOfField(rayDirection);
                }
            }

			return pixelColor * superSampleReciprocal;
		};

        //Render the pixel data into the provided output buffer using the task scheduler.
        for (u16 py = 0; py < viewportDimsInPx.y; py += clusterSize.y)
        {
            for (u16 px = 0; px < viewportDimsInPx.x; px += clusterSize.x)
            {
				//Spawn a task for the task scheduler in the form of a ray cluster.
				TaskScheduler::Task task = [&, clusterSize, px, py]()
				{
					for (u8 cpy = 0; cpy < clusterSize.y; cpy++)
					{
						for (u8 cpx = 0; cpx < clusterSize.x; cpx++)
						{
                            const Color rgb = SuperSamplePixel(vath::Vector2u32(px + cpx, py + cpy));
							const u32 pi = (px + cpx + (py + cpy) * viewportDimsInPx.x);
                            a_colorDataBuffer[pi] = LinearToSrgb(rgb);
						}
					}
				};

                m_taskScheduler.Execute(task);
            }

			DXRAY_TRACE("PixelY: {} / {}", py, viewportDimsInPx.y);
        }

		m_taskScheduler.Wait();
	}

	Color Renderer::TraceRayColor(const Ray& a_ray, const riow::Scene& a_scene, const u8 a_maxTraceDepth) const
	{
		//When max depth is reached return black.
		if (a_maxTraceDepth <= 0)
		{
			return Color(0.0f);
		}

		//Otherwise keep tracing.
		riow::IntersectionInfo hitInfo;
		if (!a_scene.DoesIntersect(a_ray, m_camera.GetZNear(), m_camera.GetZFar(), hitInfo))
		{
			//#Todo: Potentially add a skysphere here, which would replace the solid color.
			return m_backgroundColor;
		}

        Ray scattered;
        Color attenuation;

		const Color emissiveLight = hitInfo.Mat->Emitted(hitInfo.UvCoord, hitInfo.Point);
        if (!hitInfo.Mat->Scatter(a_ray, hitInfo, attenuation, scattered))
        {
			return emissiveLight; //An emissive material does not scatter, it emits, hence scatter returns false.
        }

		Color diffuseReflectance = attenuation * TraceRayColor(scattered, a_scene, a_maxTraceDepth - 1);
        return emissiveLight + diffuseReflectance;
	}
}