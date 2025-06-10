#pragma once
#include <core/containers/array.h>
#include "dxrtracer/accelerationStructure.h"

// #note_scene: Should the scene contain the tlas update? Even better, do we have to keep track of 3 Tlas, due to in-flight generation?
// #todo: add a form of scene objects/instances so the D3D12_RAYTRACING_INSTANCE_DESC can be moved into a Tlas abstraction. This will
// help with the instances book-keeping.

namespace dxray
{
	/**
	 * @brief The scene is responsible for the object instancing and top level acceleration structure generation, as this requires a rebuild each time something dynamic
	 * the scene moves, may it be the camera or the objects.
	 */
	class Scene final
	{
	public:
		Scene();
		~Scene() = default;

		void Tick(const float a_dt);

		void AddSceneObjectInstance(const AccelerationStructure& a_blasData);
		void UpdateTlas(ComPtr<ID3D12Device> a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_frameResourceData, ComPtr<ID3D12Resource>& a_instanceBuffer);

	private:
		Array<D3D12_RAYTRACING_INSTANCE_DESC> m_sceneObjectInstances;
		bool m_isDirty = false;
	};
}