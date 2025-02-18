#include "riow/traceable/sphere.h"

namespace dxray::riow
{
	Sphere::Sphere(const vath::Vector3& a_center, const fp32 a_radius, std::shared_ptr<Material> a_material) :
		m_translation(a_center, vath::Vector3f(0.0f)),
		m_radius(a_radius),
		m_material(a_material)
	{ }

    Sphere::Sphere(const vath::Vector3& a_frameStartCenter, const vath::Vector3& a_frameEndCenter, const fp32 a_radius, std::shared_ptr<Material> a_material /*= nullptr*/) :
		m_translation(a_frameStartCenter, a_frameEndCenter - a_frameStartCenter),
		m_radius(std::fmaxf(0.0f, a_radius)),
		m_material(a_material)
    { }

	bool Sphere::DoesIntersect(const Ray& a_ray, const fp32 a_tMin, fp32 const a_tMax, IntersectionInfo& a_info) const
	{
		const vath::Vector3f centerAtTime = m_translation.At(a_ray.GetTime());
		const vath::Vector3f rayFromCenter = centerAtTime - a_ray.GetOrigin();
		const fp32 a = vath::SqrMagnitude(a_ray.GetDirection());
		const fp32 h = vath::Dot(a_ray.GetDirection(), rayFromCenter);
		const fp32 c = vath::SqrMagnitude(rayFromCenter) - m_radius * m_radius;

		const fp32 discriminant = h * h - a * c;
		if (discriminant < 0.0f)
		{
			return false;
		}

		const fp32 sqrtDiscriminant = std::sqrt(discriminant);
		fp32 t = (h - sqrtDiscriminant) / a;
		if (t <= a_tMin || t > a_tMax)
		{
			t = (h + sqrtDiscriminant) / a;
			if (t <= a_tMin || t > a_tMax)
			{
				return false;
			}
		}

		a_info.Point = a_ray.At(t);
		a_info.Length = t;
		a_info.Mat = m_material;
		const vath::Vector3f outwardNormal = (a_info.Point - centerAtTime) / m_radius;
		a_info.SetFaceNormal(a_ray, outwardNormal);
		a_info.UvCoord = Sphere::PointToUv(outwardNormal);

		return true;
	}

	vath::Vector2f Sphere::PointToUv(const vath::Vector3f& a_point)
	{
		const fp32 theta = std::acos(-a_point.y);
		const fp32 phi = std::atan2(-a_point.z, a_point.x) + vath::Pi<fp32>();

		return vath::Vector2f(
			phi / (2.0f * vath::Pi<fp32>()),
			theta / vath::Pi<fp32>()
		);
	}
}