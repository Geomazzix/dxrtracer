#pragma once
#include <core/vath/vath.h>

namespace dxray::riow
{
	/// <summary>
	/// Ray class representing P = A + t(B);
	/// </summary>
	class Ray final
	{
	public:
		Ray();
		Ray(const vath::Vector3& a_origin, const vath::Vector3& a_direction, const fp32 a_time = 0);
		~Ray() = default;

		const vath::Vector3f& GetOrigin() const;
		const vath::Vector3f& GetDirection() const;
		vath::Vector3f At(fp32 a_scalar) const;

		const fp32 GetTime() const;

	private:
		vath::Vector3f m_origin;
		vath::Vector3f m_direction;
		fp32 m_time;
	};

	inline Ray::Ray() :
		m_origin(0.0f),
		m_direction(1.0f),
		m_time(0.0f)
	{}

	inline Ray::Ray(const vath::Vector3& a_origin, const vath::Vector3& a_direction, const fp32 a_time /*=0*/) :
		m_origin(a_origin),
		m_direction(a_direction),
		m_time(a_time)
	{}

	inline const vath::Vector3f& Ray::GetOrigin() const
	{
		return m_origin;
	}

	inline const vath::Vector3f& Ray::GetDirection() const
	{
		return m_direction;
	}

	inline vath::Vector3f Ray::At(fp32 a_scalar) const
	{
		return m_origin + m_direction * a_scalar;
	}

	inline const fp32 Ray::GetTime() const
	{
		return m_time;
	}
}