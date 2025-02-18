#include "riow/scene.h"

namespace dxray::riow
{
	void Scene::AddTraceable(std::shared_ptr<RayTraceable> a_pTraceable)
	{
		m_traceables.push_back(a_pTraceable);
	}

	void Scene::DeleteAll()
	{
		m_traceables.clear();
	}

	bool Scene::DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const
	{
		IntersectionInfo currentHitInfo;
		fp32 lastIntersectionMagnitude = a_tMax;

		for (const auto& raytraceable : m_traceables)
		{
			if (raytraceable->DoesIntersect(a_ray, a_tMin, lastIntersectionMagnitude, currentHitInfo))
			{
				lastIntersectionMagnitude = currentHitInfo.Length;
				a_info = currentHitInfo;
			}
		}

		return lastIntersectionMagnitude < a_tMax;
	}
}