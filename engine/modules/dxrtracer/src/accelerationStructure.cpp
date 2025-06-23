#include "dxrtracer/accelerationStructure.h"
#include "dxrtracer/modelLoader.h"

namespace dxray
{
	void CreateAccelerationStructure(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, ComPtr<ID3D12Resource>& a_buffer, ComPtr<ID3D12Resource>& a_scratch, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& a_asInputs)
	{
		auto CreateGpuBuffer = [&](const u64 a_sizeInBytes, const D3D12_RESOURCE_STATES a_initialState)
		{
			const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(a_sizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			ComPtr<ID3D12Resource> buffer;
			D3D12_CHECK(a_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, a_initialState, nullptr, IID_PPV_ARGS(&buffer)));
			return buffer;
		};

		ComPtr<ID3D12Device5> dxrDevice;
		D3D12_CHECK(a_device.As(&dxrDevice));
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
		dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&a_asInputs, &prebuildInfo);

		a_scratch = CreateGpuBuffer(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON);
		a_buffer = CreateGpuBuffer(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc =
		{
			.DestAccelerationStructureData = a_buffer->GetGPUVirtualAddress(),
			.Inputs = a_asInputs,
			.ScratchAccelerationStructureData = a_scratch->GetGPUVirtualAddress()
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
				.pResource = a_buffer.Get()
			}
		};
		dxrCmdList->ResourceBarrier(1, &asbarrier);
	}

	void CreateBlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, BottomLevelAccelerationStructure& a_blas, const ComPtr<ID3D12Resource>& a_vsBuffer, const usize a_vsCount, ComPtr<ID3D12Resource> a_idBuffer, const usize a_idCount)
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
			.pGeometryDescs = &geometryDesc,
		};

		CreateAccelerationStructure(a_device, a_cmdList, a_blas.Buffer, a_blas.Scratch, blasInputs);
	}

	void UpdateTlas(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, TopLevelAccelerationStructure& a_tlas, const Array<D3D12_RAYTRACING_INSTANCE_DESC> a_sceneObjectInstances)
	{
		const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * a_sceneObjectInstances.size());
		const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		D3D12_CHECK(a_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&a_tlas.WorldTlasInstancesData)));
		if (!a_tlas.WorldTlasInstancesData)
		{
			return;
		}

		void* mappedSceneObjectsAddr = nullptr;
		D3D12_CHECK(a_tlas.WorldTlasInstancesData->Map(0, nullptr, &mappedSceneObjectsAddr));
		D3D12_RAYTRACING_INSTANCE_DESC* const instances = static_cast<D3D12_RAYTRACING_INSTANCE_DESC* const>(mappedSceneObjectsAddr);

		for (usize descIdx = 0; descIdx < a_sceneObjectInstances.size(); descIdx++)
		{
			instances[descIdx] = a_sceneObjectInstances[descIdx];
		}

		if (mappedSceneObjectsAddr)
		{
			a_tlas.WorldTlasInstancesData->Unmap(0, nullptr);
		}

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs =
		{
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
			.NumDescs = static_cast<u32>(a_sceneObjectInstances.size()),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = a_tlas.WorldTlasInstancesData->GetGPUVirtualAddress()
		};

		CreateAccelerationStructure(a_device, a_cmdList, a_tlas.Buffer, a_tlas.Scratch, tlasInputs);

		D3D12_NAME_OBJECT(a_tlas.Scratch, std::format(L"WorldTlasScratch"));
		D3D12_NAME_OBJECT(a_tlas.Buffer, std::format(L"WorldTlas"));
	}
}