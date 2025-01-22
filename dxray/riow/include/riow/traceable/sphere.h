#pragma once
#include "riow/traceable/raytraceable.h"
#include "riow/material.h"

namespace dxray::riow
{
	/// <summary>
	/// An algebraic representation of a sphere, traceable.
	/// </summary>
	class Sphere final : public RayTraceable
	{
	public:
		Sphere(const vath::Vector3& a_center, fp32 a_radius, std::shared_ptr<Material> a_material = nullptr);
		~Sphere() = default;

		bool DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const override;
		void SetMaterial(std::shared_ptr<Material> a_material);

	private:
		vath::Vector3 m_center;
		fp32 m_radius;
		std::shared_ptr<Material> m_material;
	};

	inline void Sphere::SetMaterial(std::shared_ptr<Material> a_material)
	{
		m_material = a_material;
	}
}