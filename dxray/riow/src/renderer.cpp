#include "riow/renderer.h"
#include "riow/material.h"

namespace dxray::riow
{
	void Renderer::Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer)
	{
		//Retrieving all data needed for render.
		const vath::Vector3f cameraPosition = m_camera.GetPosition();
		const vath::Vector2u32 viewportDimensionsInPx = m_camera.GetViewportDimensionsInPx();
		const vath::Vector2f viewportDimensions(2.0f * m_camera.GetAspectRatio(), 2.0f);
		const fp32 focalLength = m_camera.GetFocalLength();

		//#Todo: simplify this math, it's confusing to use vectors like this, with SIMD performance would be equivalent, though from a programmer's POV it's hard to read.
		//Calculating the image plane to shoot rays through.
		const vath::Vector3f viewportU(viewportDimensions.x, 0.0f, 0.0f);
		const vath::Vector3f viewportV(0.0f, -viewportDimensions.y, 0.0f);
		const vath::Vector3f pixelDeltaU(viewportU / static_cast<fp32>(viewportDimensionsInPx.x));
		const vath::Vector3f pixelDeltaV(viewportV / static_cast<fp32>(viewportDimensionsInPx.y));

		const vath::Vector3f viewportUpperLeft = cameraPosition - vath::Vector3f(0.0f, 0.0f, focalLength) - viewportU / 2.0f - viewportV / 2.0f;
		const vath::Vector3f pixelCenter = 0.5f * (pixelDeltaU + pixelDeltaV);

		const u8 sampleSize = m_pipelineConfiguration.AASampleCount;
		const fp32 sampleReciprocal = 1.0f / static_cast<fp32>(sampleSize * sampleSize);

		//Sample the pixel index provided, using the values defined above.
		auto SamplePixel = [=](const vath::Vector2u32& a_pixelIndex)
		{
            Color pixelColor(0.0f);
            for (u8 sy = 0; sy < sampleSize; sy++)
            {
                for (u8 sx = 0; sx < sampleSize; sx++)
                {
                    const fp32 r = vath::RandomNumber<fp32>();
                    const vath::Vector2f sampleOffset((sx + r) / sampleSize, (sy + r) / sampleSize);

                    const vath::Vector3f pixelPosition = viewportUpperLeft + pixelCenter +
                        ((a_pixelIndex.x + sampleOffset.x) * pixelDeltaU) +
                        ((a_pixelIndex.y + sampleOffset.y) * pixelDeltaV);
                    const vath::Vector3f rayDirection = pixelPosition - cameraPosition;

                    const riow::Ray camRay(cameraPosition, rayDirection);
                    pixelColor += TraceRayColor(camRay, a_scene, m_pipelineConfiguration.MaxTraceDepth);
                }
            }

			return pixelColor * sampleReciprocal;
		};

        //Render the pixel data into the provided output buffer.
        const u32 pixelCount = viewportDimensionsInPx.y * viewportDimensionsInPx.x;
        for (u32 pixelIndex = 0, pixely = 0; pixely < viewportDimensionsInPx.y; pixely++)
        {
            for (u32 pixelx = 0; pixelx < viewportDimensionsInPx.x; pixelx++, pixelIndex++)
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