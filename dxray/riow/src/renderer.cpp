#include "riow/renderer.h"

namespace dxray::riow
{
	void Renderer::Render(const Scene& a_scene, std::vector<vath::Vector3f>& a_colorDataBuffer)
	{
		//Retrieving all data needed for render.
		const vath::Vector3f cameraPosition = m_camera.GetPosition();
		const vath::Vector2u32 viewportDimensionsInPx = m_camera.GetViewportDimensionsInPx();
		const vath::Vector2f viewportDimensions(2.0f * m_camera.GetAspectRatio(), 2.0f);
		const fp32 focalLength = m_camera.GetFocalLength();

		//#Todo: simplify this math, it's confusing to use vectors like this, with SIMD performance would be equivalant, though from a programmer's POV it's hard to read.
		//Calculating the image plane to shoot rays through.
		const vath::Vector3f viewportU(viewportDimensions.x, 0.0f, 0.0f);
		const vath::Vector3f viewportV(0.0f, -viewportDimensions.y, 0.0f);
		const vath::Vector3f pixelDeltaU(viewportU / static_cast<fp32>(viewportDimensionsInPx.x));
		const vath::Vector3f pixelDeltaV(viewportV / static_cast<fp32>(viewportDimensionsInPx.y));

		const vath::Vector3f viewportUpperLeft = cameraPosition - vath::Vector3f(0.0f, 0.0f, focalLength) - viewportU / 2.0f - viewportV / 2.0f;
		const vath::Vector3f pixelCenter = 0.5f * (pixelDeltaU + pixelDeltaV);

		const u8 sampleSize = m_pipelineConfiguration.AASampleCount;

		//Rendering the data into the color buffer.
		for (u32 pixely = 0; pixely < viewportDimensionsInPx.y; pixely++)
		{
			for (u32 pixelx = 0; pixelx < viewportDimensionsInPx.x; pixelx++)
			{
				//Anti-aliasing - #Todo: see if this can be moved into a seperate function to reduce complexity.
				vath::Vector3f pixelColor(0.0f);
				for (u8 sy = 0; sy < sampleSize; sy++)
				{
					for (u8 sx = 0; sx < sampleSize; sx++)
					{
						//#Note: Double check on validity: tiger book says:
						// color = rayColor(pixelx + 0.5, pixely + 0.5) for each pixel without aa.
						//while with aa it turns:
						// color += rayColor(pixelx + (samplex + r) / sampleSize, pixely + (sampley + r) / sampleSize).
						//The current algorithm offsets this a bit differently, which could lead to the wrong pixel positions.

						const fp32 r = vath::RandomNumber<fp32>();
						const vath::Vector3f pixelPosition = viewportUpperLeft + pixelCenter +
							((pixelx + (sx + r) / sampleSize) * pixelDeltaU) +
							((pixely + (sy + r) / sampleSize) * pixelDeltaV);
						const vath::Vector3f rayDirection = pixelPosition - cameraPosition;

						const riow::Ray camRay(cameraPosition, rayDirection);
						pixelColor += ComputeRayColor(camRay, a_scene);
					}
				}

				const u32 pixelIndex = pixelx + pixely * viewportDimensionsInPx.x;
				a_colorDataBuffer[pixelIndex] = pixelColor / static_cast<fp32>(sampleSize * sampleSize);
			}
		}
	}

	vath::Vector3 Renderer::ComputeRayColor(const riow::Ray& a_ray, const riow::Scene& a_scene)
	{
		riow::IntersectionInfo hitInfo;
		if (a_scene.DoesIntersect(a_ray, m_camera.GetZNear(), m_camera.GetZFar(), hitInfo))
		{
			return 0.5f * (hitInfo.Normal + 1.0f); //#Note: Temporarily, currently used to visualize normals.
		}

		//If no intersection took place render a sky-like gradient, emulated through a simple lerp.
		const fp32 a = 0.5f * (Normalize(a_ray.GetDirection()).y + 1.0f);
		return (1.0f - a) * vath::Vector3(1.0f) + a * vath::Vector3(0.5f, 0.7f, 1.0f);
	}
}