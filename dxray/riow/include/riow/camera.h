#pragma once

namespace dxray::riow
{
	//#Todo: Check for FoV, scale and DoF properties.
	//#Todo: Add a transform on the camera so it can support 3rd scene hierarchies.

	/// <summary>
	/// The camera functions as configuration and spatial position to render the scene from in the renderer.
	/// </summary>
	class Camera final
	{
	public:
		Camera();
		~Camera() = default;

		void SetViewportDimensionInPx(const vath::Vector2u32& a_viewportDimensionInPx);

		void SetDepthLimits(const vath::Vector2f& a_depthLimit);
		void SetZNear(const fp32 a_zNear);
		void SetZFar(const fp32 a_zFar);
		void SetFocalLength(const fp32 a_focalLength);

		const vath::Vector3f GetPosition() const;
		const vath::Vector2u32 GetViewportDimensionsInPx() const;
		const u32 GetViewportWidthInPx() const;
		const u32 GetViewportHeightInPx() const;
		const vath::Vector2f GetDepthLimits() const;
		const fp32 GetZNear() const;
		const fp32 GetZFar() const;
		const fp32 GetAspectRatio() const;
		const fp32 GetFocalLength() const;

	private:
		vath::Vector3 m_position;
		vath::Vector2u32 m_viewportPixelDims;
		vath::Vector2f m_depthLimits;
		fp32 m_aspectRatio;
		fp32 m_focalLength;
	};


	inline void Camera::SetDepthLimits(const vath::Vector2f& a_depthLimit)
	{
		m_depthLimits = a_depthLimit;
	}

	inline void Camera::SetZNear(const fp32 a_zNear)
	{
		m_depthLimits.x = a_zNear;
	}

	inline void Camera::SetZFar(const fp32 a_zFar)
	{
		m_depthLimits.y = a_zFar;
	}

	inline void Camera::SetFocalLength(const fp32 a_focalLength)
	{
		m_focalLength = a_focalLength;
	}

	inline const vath::Vector3f Camera::GetPosition() const
	{
		return m_position;
	}

	inline const vath::Vector2u32 Camera::GetViewportDimensionsInPx() const
	{
		return m_viewportPixelDims;
	}

	inline const u32 Camera::GetViewportWidthInPx() const
	{
		return m_viewportPixelDims.x;
	}

	inline const u32 Camera::GetViewportHeightInPx() const
	{
		return m_viewportPixelDims.y;
	}

	inline const vath::Vector2f Camera::GetDepthLimits() const
	{
		return m_depthLimits;
	}

	inline const fp32 Camera::GetZNear() const
	{
		return m_depthLimits.x;
	}

	inline const fp32 Camera::GetZFar() const
	{
		return m_depthLimits.y;
	}

	inline const fp32 Camera::GetAspectRatio() const
	{
		return m_aspectRatio;
	}

	inline const fp32 Camera::GetFocalLength() const
	{
		return m_focalLength;
	}
}