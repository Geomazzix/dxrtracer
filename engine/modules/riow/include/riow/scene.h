#pragma once
#include "riow/traceable/raytraceable.h"

namespace dxray::riow
{
	/// <summary>
	/// Responsible for the lifetime and tracing of *raytracable* objects.
	/// Also contains all light and BVH information of the scene.
	/// #Note: No individual removal of objects is possible/needed as the functionality of riow is to render the scene once into a static image.
	/// #Note: The name *world* would be more appropriate here, as one always exists, though this adds the complexity of adding *scenes* to a world, which is overscoped,
	/// as serialization is not a topic for riow.
	/// </summary>
	class Scene final
	{
	public:
		Scene() = default;
		~Scene() = default;

		void AddTraceable(std::shared_ptr<RayTraceable> a_pTraceable);
		void DeleteAll();

		bool DoesIntersect(const Ray& a_ray, fp32 a_tMin, fp32 a_tMax, IntersectionInfo& a_info) const;

	private:
		std::vector<std::shared_ptr<RayTraceable>> m_traceables;
	};
}