#pragma once
#include "riow/ray.h"

namespace dxray::riow
{
	/// <summary>
	/// Structure containing the intersection results on a ray-traceable object.
	/// </summary>
	struct IntersectionInfo final
	{
		vath::Vector3f Point = vath::Vector3(0.0f);
		vath::Vector3f Normal = vath::Vector3(0.0f);
		fp32 Length = 0.0f;
		bool FrontFace = false;

		//#Todo: move this to where applicable - probably only in refractive/volumetric volumes.
		void SetFaceNormal(const Ray& a_ray, const vath::Vector3& a_outwardNormal);
	};
	
	inline void IntersectionInfo::SetFaceNormal(const Ray& a_ray, const vath::Vector3& a_outwardNormal)
	{
		FrontFace = vath::Dot(a_ray.GetDirection(), a_outwardNormal) < 0.0f;
		Normal = FrontFace ? a_outwardNormal : -a_outwardNormal;
	}


	/// <summary>
	/// Interface for anything a ray is able to detect intersections with.
	/// </summary>
	class RayTraceable
	{
	public:
		virtual ~RayTraceable() = default;
		virtual bool DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const = 0;
	};
}