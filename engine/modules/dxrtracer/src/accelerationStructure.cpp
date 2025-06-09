#include "dxrtracer/accelerationStructure.h"
#include "dxrtracer/modelLoader.h"

namespace dxray
{
	void CreateAccelerationStructure(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& a_asInputs)
	{
		auto CreateGpuBuffer = [&](u64 a_sizeInBytes, D3D12_RESOURCE_STATES a_initialState)
		{
			const D3D12_RESOURCE_DESC resourceDesc =
			{
				.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
				.Alignment = 0,
				.Width = a_sizeInBytes,
				.Height = 1,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_UNKNOWN,
				.SampleDesc = { 1, 0 },
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
				.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			};

			const D3D12_HEAP_PROPERTIES heapProps =
			{
				.Type = D3D12_HEAP_TYPE_DEFAULT
			};

			ComPtr<ID3D12Resource> resource;
			D3D12_CHECK(a_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, a_initialState, nullptr, IID_PPV_ARGS(&resource)));
			return resource;
		};

		ComPtr<ID3D12Device5> dxrDevice;
		D3D12_CHECK(a_device.As(&dxrDevice));
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
		dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&a_asInputs, &prebuildInfo);

		a_as.Scratch = CreateGpuBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON);
		a_as.Buffer = CreateGpuBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc =
		{
			.DestAccelerationStructureData = a_as.Buffer->GetGPUVirtualAddress(),
			.Inputs = a_asInputs,
			.ScratchAccelerationStructureData = a_as.Scratch->GetGPUVirtualAddress()
		};

		ComPtr<ID3D12GraphicsCommandList5> dxrCmdList;
		D3D12_CHECK(a_cmdList.As(&dxrCmdList));
		dxrCmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		//#Todo: once queuing for multiple builds ensure that multiple barriers are batched.
		const D3D12_RESOURCE_BARRIER asbarrier =
		{
			.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
			.UAV =
			{
				.pResource = a_as.Buffer.Get()
			}
		};
		dxrCmdList->ResourceBarrier(1, &asbarrier);
	}

	void CreateBlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const ComPtr<ID3D12Resource>& a_vsBuffer, const usize a_vsCount, ComPtr<ID3D12Resource> a_idBuffer, const usize a_idCount)
	{
		const D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE vertexBuffer =
		{
			.StartAddress = a_vsBuffer->GetGPUVirtualAddress(),
			.StrideInBytes = Vertex::Stride
		};

		const D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC triangleDesc =
		{
			.Transform3x4 = 0,
			.IndexFormat = a_idCount > 0 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN,
			.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
			.IndexCount = a_idCount > 0 ? static_cast<unsigned>(a_idCount) : 0,
			.VertexCount = static_cast<unsigned>(a_vsCount),
			.IndexBuffer = a_idBuffer ? a_idBuffer->GetGPUVirtualAddress() : 0,
			.VertexBuffer = vertexBuffer
		};

		//#Todo: add translucency support.
		const D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc =
		{
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
			.Triangles = triangleDesc
		};

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs =
		{
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = &geometryDesc
		};

		CreateAccelerationStructure(a_device, a_cmdList, a_as, blasInputs);
	}
}