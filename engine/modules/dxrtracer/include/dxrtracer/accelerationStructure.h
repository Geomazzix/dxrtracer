#pragma once

/*#Todo: Abstract acceleration structures into a scalable world hierarchy:
World (geometry data):
- Tlas (main all-owning Tlas)
	- Tlas (scene/chunk tlas - depending on whether world partitioning is a thing and whether depth based updating is a thing - see Cascaded Shadow Map update rates.)
		- Blas (x-amount per Tlas *should* be reusable. Note that the way these are constructed is heavily dependant on the type of geometry, see D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS.)

- Each Blas should be assigned to a mesh/model (or multiple, depending on the level of abstraction/optimization).
- Each Tlas should be owned by the world and managed by the world, update on the CPU, rebuild on the GPU.
- Each mesh/model has indices/handles that can be used to identify and set the PSO/rootsigs accordingly.
*/

namespace dxray
{
	struct AccelerationStructure
	{
		ComPtr<ID3D12Resource> Buffer = nullptr;
		ComPtr<ID3D12Resource> Scratch = nullptr; //#Note: scratch memory is used during the building of the BVH. After this is can be reused for other purposes - source: do and don't from nvidia.
	};

	extern void CreateAccelerationStructure(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& a_asInputs);
	extern void CreateBlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const ComPtr<ID3D12Resource>& a_vsBuffer, const usize a_vsCount, ComPtr<ID3D12Resource> a_idBuffer = nullptr, const usize a_idCount = 0);
}