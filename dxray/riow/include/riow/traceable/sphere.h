#pragma once
#include "riow/traceable/raytraceable.h"

namespace dxray::riow
{
	/// <summary>
	/// An algebraic represention of a sphere, traceable.
	/// </summary>
	class Sphere final : public RayTraceable
	{
	public:
		Sphere(const vath::Vector3& a_center, fp32 a_radius);
		~Sphere() = default;

		bool DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const override;

	private:
		vath::Vector3 m_center;
		fp32 m_radius;
	};
}