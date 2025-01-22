#include "riow/traceable/sphere.h"

namespace dxray::riow
{
	Sphere::Sphere(const vath::Vector3& a_center, fp32 a_radius, std::shared_ptr<Material> a_material) :
		m_center(a_center),
		m_radius(a_radius),
		m_material(a_material)
	{ }

	bool Sphere::DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const
	{
		vath::Vector3f rayFromCenter = m_center - a_ray.GetOrigin();
		const fp32 a = vath::SqrMagnitude(a_ray.GetDirection());
		const fp32 h = vath::Dot(a_ray.GetDirection(), rayFromCenter);
		const fp32 c = vath::SqrMagnitude(rayFromCenter) - m_radius * m_radius;

		const fp32 discriminant = h * h - a * c;
		if (discriminant < 0)
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
		vath::Vector3f outwardNormal = (a_info.Point - m_center) / m_radius;
		a_info.SetFaceNormal(a_ray, outwardNormal);
		a_info.Mat = m_material;

		return true;
	}
}