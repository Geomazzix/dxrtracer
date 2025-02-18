#include "riow/camera.h"

namespace dxray::riow
{
	Camera::Camera() :
		m_worldTransform(),
		m_viewportPixelDims(0u, 0u),
		m_depthLimits(0.001f, 1000.0f),
		m_aspectRatio(0.0f),
		m_focalLength(1.0f),
		m_aperture(0.0f),
		m_fov(90.0f),
		m_shutterSpeed(1.0f)
	{}

	void Camera::SetViewportDimensionInPx(const vath::Vector2u32& a_viewportDimensionInPx)
	{
		m_viewportPixelDims = a_viewportDimensionInPx;
		m_aspectRatio = static_cast<fp32>(a_viewportDimensionInPx.x) / a_viewportDimensionInPx.y;
	}

    void Camera::LookAt(const vath::Vector3& a_position, const vath::Vector3f a_focusPoint, const vath::Vector3f& a_worldNormal /*= vath::Vector3f(0.0f, 1.0f, 0.0f)*/)
    {
		DXRAY_ASSERT_WITH_MSG(vath::SqrMagnitude(m_viewportPixelDims) > 0.0f, "Ensure that the viewport pixel dimensions are set -> Camera::SetViewportDimensionInPx");

		m_worldTransform = vath::LookAtRH(a_position, a_focusPoint, a_worldNormal);
        const fp32 imagePlaneScale = std::tanf(m_fov / 2.0f);

		//Calculate the viewport rect based on the look at position. It's always centered on the camera position.
		//#Note: Negate the image-plane here, thereby removing this factor from the ray generation algorithm.
		m_viewportRect.Height = -2.0f * imagePlaneScale;
        m_viewportRect.Width = 2.0f * imagePlaneScale * m_aspectRatio;
		m_viewportRect.x = -m_viewportRect.Width * 0.5f;
		m_viewportRect.y = -m_viewportRect.Height * 0.5f;
	}
}