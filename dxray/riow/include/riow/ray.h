#pragma once
#include <core/vath/vath.h>

namespace dxray::riow
{
	/// <summary>
	/// Ray class representing P = A + t(B);
	/// </summary>
	class Ray
	{
	public:
		Ray();
		Ray(const vath::Vector3& a_origin, const vath::Vector3& a_direction);
		~Ray() = default;

		const vath::Vector3f& GetOrigin() const;
		const vath::Vector3f& GetDirection() const;
		vath::Vector3f At(fp32 a_scalar) const;

	private:
		const vath::Vector3f m_origin;
		const vath::Vector3f m_direction;
	};

	inline Ray::Ray() :
		m_origin(0.0f),
		m_direction(1.0f)
	{}

	inline Ray::Ray(const vath::Vector3& a_origin, const vath::Vector3& a_direction) :
		m_origin(a_origin),
		m_direction(a_direction)
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
}