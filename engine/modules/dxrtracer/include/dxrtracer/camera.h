#pragma once
#include <core/vath/matrix4x4.h>

namespace dxray
{
	/*!
	 * @brief Camera struct handles view-projection generation data.
	 */
	struct Camera
	{
		vath::Vector3f Position;
		fp32 AspectRatio;
		vath::Vector3f Up;
		fp32 Fov;
		vath::Vector3f Right;
		fp32 ZNear;
		vath::Vector3f Forward;
		fp32 ZFar;

		Camera(const fp32 a_fov, const fp32 a_aspectRatio, const fp32 a_zNear = 0.1f, const fp32 a_zFar = 1000.0f) :
			AspectRatio(a_aspectRatio),
			Fov(a_fov),
			ZNear(a_zNear),
			ZFar(a_zFar)
		{
			m_projection = vath::PerspectiveFovRH(Fov, AspectRatio, ZNear, ZFar);
		}

		~Camera() = default;

		inline void LookAt(const vath::Vector3& a_position, const vath::Vector3f& a_lookat, const vath::Vector3f& a_worldUp = vath::Vector3f(0.0f, 1.0f, 0.0f))
		{
			Position = a_position;
			m_view = vath::LookAtRH(Position, a_lookat, a_worldUp);
		}

		inline const vath::Matrix4x4f GetViewMatrix() const
		{
			return m_view;
		}
		
		inline const vath::Matrix4x4f GetProjectionMatrix() const
		{
			return m_projection;
		}

	private:
		vath::Matrix4x4f m_view;
		vath::Matrix4x4f m_projection;
	};
}