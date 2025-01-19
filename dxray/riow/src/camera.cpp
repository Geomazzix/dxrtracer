#include "riow/camera.h"

namespace dxray::riow
{
	Camera::Camera() :
		m_position(0.0f),
		m_viewportPixelDims(0u, 0u),
		m_depthLimits(0.0f, 0.0f),
		m_aspectRatio(0.0f),
		m_focalLength(1.0f)
	{}

	void Camera::SetViewportDimensionInPx(const vath::Vector2u32& a_viewportDimensionInPx)
	{
		m_viewportPixelDims = a_viewportDimensionInPx;
		m_aspectRatio = static_cast<fp32>(a_viewportDimensionInPx.x) / a_viewportDimensionInPx.y;
	}
}