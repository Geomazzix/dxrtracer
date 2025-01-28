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
		Sphere(const vath::Vector3& a_center, const fp32 a_radius, std::shared_ptr<Material> a_material = nullptr);
		Sphere(const vath::Vector3& a_frameStartCenter, const vath::Vector3& a_frameEndCenter, const fp32 a_radius, std::shared_ptr<Material> a_material = nullptr);
		~Sphere() = default;

		bool DoesIntersect(const Ray& a_ray, const fp32 a_tMin, const fp32 a_tMax, IntersectionInfo& a_info) const override;
		void SetMaterial(std::shared_ptr<Material> a_material);

		static vath::Vector2f PointToUv(const vath::Vector3f& a_point);

	private:
		Ray m_translation;
		fp32 m_radius;
		std::shared_ptr<Material> m_material;
	};

	inline void Sphere::SetMaterial(std::shared_ptr<Material> a_material)
	{
		m_material = a_material;
	}
}