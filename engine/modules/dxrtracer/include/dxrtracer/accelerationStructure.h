#pragma once

// #Todo_accelerationStructure: add copy commands for serialization/deserialization, meaning the Tlas/Blas can be saved to disk
// thereby re-purposed for application launch (i.e. an object might only need their blas, vertex/index buffer and transform), while
// the Tlas could be stored in a world/scene file.

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