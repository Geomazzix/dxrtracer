#pragma once

namespace dxray::riow
{
	//#Todo: Check for FoV, scale and DoF properties.
	//#Todo: Add a transform on the camera so it can support 3rd scene hierarchies.

	/// <summary>
	/// The camera functions as configuration and spatial position to render the scene from in the renderer.
	/// It's also responsible for the viewport calculations.
	/// </summary>
	class Camera final
	{
	public:
		Camera();
		~Camera() = default;

		void SetViewportDimensionInPx(const vath::Vector2u32& a_viewportDimensionInPx);
		void LookAt(const vath::Vector3& a_position, const vath::Vector3f a_focusPoint, const vath::Vector3f& a_worldNormal = vath::Vector3f(0.0f, 1.0f, 0.0f));
		void SetDepthLimits(const vath::Vector2f& a_depthLimit);
		void SetZNear(const fp32 a_zNear);
		void SetZFar(const fp32 a_zFar);
		void SetVerticalFov(const fp32 a_vfovInRad);
		void SetFocalLength(const fp32 a_focusDistance);
		void SetAperture(const fp32 a_focusDistance);
		void SetShutterSpeed(const fp32 a_shutterSpeedInSec);

		const vath::Vector3f GetPosition() const;
		const vath::Vector2u32 GetViewportDimensionsInPx() const;
		const u32 GetViewportWidthInPx() const;
		const u32 GetViewportHeightInPx() const;
		const vath::Vector2f GetDepthLimits() const;
		const fp32 GetZNear() const;
		const fp32 GetZFar() const;
		const fp32 GetAspectRatio() const;
		const fp32 GetFocalLength() const;
		const fp32 GetAperture() const;
		const fp32 GetFov() const;
		const fp32 GetShutterSpeed() const;
		const vath::Rect<fp32> GetViewportRect() const;
		const vath::Matrix4x4f GetWorldTransform() const;

	private:
		vath::Matrix4x4f m_worldTransform;
		vath::Vector2u32 m_viewportPixelDims;
		vath::Rect<fp32> m_viewportRect;
		vath::Vector2f m_depthLimits;
		fp32 m_aspectRatio;
		fp32 m_focalLength;
		fp32 m_aperture;
		fp32 m_fov;
		fp32 m_shutterSpeed;
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

	inline void Camera::SetVerticalFov(const fp32 a_vfovInRad)
	{
		m_fov = a_vfovInRad;
	}

	inline void Camera::SetFocalLength(const fp32 a_focalLength)
	{
		m_focalLength = a_focalLength;
	}

	inline void Camera::SetAperture(const fp32 a_aperture)
	{
		m_aperture = a_aperture;
	}

	inline void Camera::SetShutterSpeed(const fp32 a_shutterSpeedInSec)
	{
		m_shutterSpeed = a_shutterSpeedInSec;
	}

	inline const vath::Vector3f Camera::GetPosition() const
	{
		const vath::Vector4& camPos = m_worldTransform[3];
		return vath::Vector3f(camPos.x, camPos.y, camPos.z);
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

	inline const fp32 Camera::GetAperture() const
	{
		return m_aperture;
	}

	inline const fp32 Camera::GetFov() const
	{
		return m_fov;
	}

	inline const fp32 Camera::GetShutterSpeed() const
	{
		return m_shutterSpeed;
	}

	inline const vath::Rect<fp32> Camera::GetViewportRect() const
	{
		return m_viewportRect;
	}

	inline const vath::Matrix4x4f Camera::GetWorldTransform() const
	{
		return m_worldTransform;
	}
}