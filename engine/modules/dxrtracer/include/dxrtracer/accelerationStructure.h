#pragma once
#include <core/containers/array.h>

// #Todo_accelerationStructure: Serialization - saving them to disk and loading them during initialization *might* save time on launch.

namespace dxray
{
	/**
	 * @brief A representation for a blas.
	 */
	struct BottomLevelAccelerationStructure
	{
		ComPtr<ID3D12Resource> Buffer = nullptr;
		ComPtr<ID3D12Resource> Scratch = nullptr; //#Note: scratch memory is used during the building of the BVH. After this is can be reused for other purposes - source: do and don't from nvidia.
	};

	/**
	 * @brief A representation for the tlas.
	 */
	struct TopLevelAccelerationStructure
	{
		ComPtr<ID3D12Resource> Buffer = nullptr;
		ComPtr<ID3D12Resource> Scratch = nullptr;
		ComPtr<ID3D12Resource> WorldTlasInstancesData = nullptr;
	};

	/**
	 * @brief Generates an acceleration structure through the defined input.
	 * @param a_device A valid DX12 device capable of dxr ray tracing.
	 * @param a_cmdList A valid DX12 command list, which supports the GraphicsCommandList5 interface.
	 * @param a_buffer An empty resource (returned as output).
	 * @param a_scratch An empty resource (returned as output).
	 * @param a_asInputs A valid defined description for the acceleration structure that's being generated.
	 */
	extern void CreateAccelerationStructure(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, ComPtr<ID3D12Resource>& a_buffer, ComPtr<ID3D12Resource>& a_scratch, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& a_asInputs);
	
	/**
	 * @brief Generates a bottom level acceleration structure based on the provided geometry.
	 * @param a_device A valid DX12 device capable of dxr ray tracing.
	 * @param a_cmdList A valid DX12 command list, which supports the GraphicsCommandList5 interface.
	 * @param a_blas An blas object, returns valid acceleration structure.
	 * @param a_vsBuffer A valid vertex buffer.
	 * @param a_vsCount The vertex count of the a_vsBuffer in bytes.
	 * @param a_idBuffer A valid index buffer, covering the vertices in the vertex.
	 * @param a_idCount The amount of indicies of the a_idBuffer in bytes.
	 */
	extern void CreateBlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, BottomLevelAccelerationStructure& a_blas, const ComPtr<ID3D12Resource>& a_vsBuffer, const usize a_vsCount, ComPtr<ID3D12Resource> a_idBuffer = nullptr, const usize a_idCount = 0);
	
	/**
	 * @brief Updates the proided tlas.
	 * @param a_device A valid DX12 device capable of dxr ray tracing.
	 * @param a_cmdList A valid DX12 command list, which supports the GraphicsCommandList5 interface.
	 * @param a_tlas An empty/valid top level acceleration structure.
	 * @param a_sceneObjectInstances a buffer pointing to the instances to be included in the top level acceleration structure.
	 */
	extern void UpdateTlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, TopLevelAccelerationStructure& a_tlas, const Array<D3D12_RAYTRACING_INSTANCE_DESC> a_sceneObjectInstances);
}