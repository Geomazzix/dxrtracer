#include "riow/renderer.h"
#include "riow/material.h"

namespace dxray::riow
{
	void Renderer::Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer)
	{
		//Retrieving all data needed for render.
		const vath::Vector3f cameraPosition = m_camera.GetPosition();
		const vath::Vector2u32 viewportDimsInPx = m_camera.GetViewportDimensionsInPx();
		const vath::Rect<fp32> viewportRect = m_camera.GetViewportRect();
        const fp32 focalLength = m_camera.GetFocalLength();

		//Calculating the image plane to shoot rays through.
		const vath::Vector2f pixelDelta(
			viewportRect.Width / static_cast<fp32>(viewportDimsInPx.x), 
			viewportRect.Height / static_cast<fp32>(viewportDimsInPx.y)
		);
		const vath::Vector2f pixelCenter = pixelDelta * 0.5f;

		const u8 sampleSize = m_pipelineConfiguration.AASampleCount;
		const u8 sampleCount = sampleSize * sampleSize;
        const fp32 pixelSampleSize = static_cast<fp32>(sampleSize) / sampleCount;
		const fp32 sampleReciprocal = 1.0f / static_cast<fp32>(sampleCount);

        DXRAY_INFO("=================================");
        DXRAY_INFO("Loaded render pipeline:");
        DXRAY_INFO("Image dimensions: {}, {}", viewportDimsInPx.x, viewportDimsInPx.y);
        DXRAY_INFO("AA-SampleSize {}", sampleSize);
        DXRAY_INFO("DoF-SampleSize {}", 0);
        DXRAY_INFO("=================================");
        DXRAY_INFO("Rendering...");

		//Sample the pixel index provided, using the values defined above.
		auto SamplePixel = [=](const vath::Vector2u32& a_pixelIndex)
		{
            Color pixelColor(0.0f);
            for (u32 sy = 1; sy <= sampleSize; ++sy)
            {
                for (u32 sx = 1; sx <= sampleSize; ++sx)
                {
                    const fp32 r = vath::RandomNumber<fp32>();
                    const vath::Vector2f sampleOffset(
						static_cast<fp32>(sx) / sampleSize - pixelSampleSize * r,
						static_cast<fp32>(sy) / sampleSize - pixelSampleSize * r
					);

					const vath::Vector2f pixelOffset(
						a_pixelIndex.x * pixelDelta.x + sampleOffset.x * pixelDelta.x,
						a_pixelIndex.y * pixelDelta.y + sampleOffset.y * pixelDelta.y
                    );

					const vath::Vector3f rayDirection = vath::Vector3f(m_camera.GetWorldTransform() * vath::Vector4f(
						viewportRect.x + pixelOffset.x,
						viewportRect.y + pixelOffset.y,
						focalLength,
						0.0f
					));

                    const riow::Ray camRay(cameraPosition, rayDirection);
                    pixelColor += TraceRayColor(camRay, a_scene, m_pipelineConfiguration.MaxTraceDepth);
                }
            }

			return pixelColor * sampleReciprocal;
		};

        //Render the pixel data into the provided output buffer.
		//#Note: the image is rendered inversed on the y-axis.
        for (u32 pixelIndex = 0, pixely = viewportDimsInPx.y; pixely > 0 ; pixely--)
        {
            for (u32 pixelx = 0; pixelx < viewportDimsInPx.x; pixelx++, pixelIndex++)
            {
				const Color rgb = SamplePixel(vath::Vector2u32(pixelx, pixely));
				a_colorDataBuffer[pixelIndex] = LinearToSrgb(rgb);
            }
        }
	}

	Color Renderer::TraceRayColor(const riow::Ray& a_ray, const riow::Scene& a_scene, const u8 a_maxTraceDepth) const
	{
		//When max depth is reached return black.
		if (a_maxTraceDepth <= 0)
		{
			return Color(0.0f);
		}

		//Otherwise keep tracing.
		riow::IntersectionInfo hitInfo;
		if (a_scene.DoesIntersect(a_ray, m_camera.GetZNear(), m_camera.GetZFar(), hitInfo))
		{
			Ray scattered;
			Color attenuation;

			if (hitInfo.Mat->Scatter(a_ray, hitInfo, attenuation, scattered))
			{
				return attenuation * TraceRayColor(scattered, a_scene, a_maxTraceDepth - 1);
			}

			return Color(0.0f);
        }

		//If no intersection took place return a blue gradient, because of the trace depth this will result in color contribution from the sky onto objects.
		const fp32 a = 0.5f * (Normalize(a_ray.GetDirection()).y + 1.0f);
		return (1.0f - a) * Color(1.0f) + a * Color(0.5f, 0.7f, 1.0f);
	}
}