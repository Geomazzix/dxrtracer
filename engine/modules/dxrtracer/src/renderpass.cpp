#include "dxrtracer/renderpass.h"
#include "dxrtracer/shaderCompiler.h"

namespace dxray
{
	RenderPass::RenderPass(ComPtr<ID3D12Device> a_device) :
		m_device(a_device)
	{
		CreateRayTraceDemoRootSig();
		CreateRayTracingPipelineStateObject();
		CreateShaderTable();
	}

	void RenderPass::Execute(ComPtr<ID3D12GraphicsCommandList>& a_commandList, const RenderPassExecuteInfo& a_execInfo)
	{
		ComPtr<ID3D12GraphicsCommandList5> dxrCmdList;
		D3D12_CHECK(a_commandList.As(&dxrCmdList));

		dxrCmdList->SetPipelineState1(m_rtpso.Pso.Get());
		dxrCmdList->SetComputeRootSignature(m_rootSig.Get());
		dxrCmdList->SetDescriptorHeaps(1, a_execInfo.UavHeap.GetAddressOf());

		const u32 descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		const CD3DX12_GPU_DESCRIPTOR_HANDLE uavTable(a_execInfo.UavHeap->GetGPUDescriptorHandleForHeapStart(), a_execInfo.SwapchainIndex, descriptorSize);
		dxrCmdList->SetComputeRootDescriptorTable(0, uavTable);
		dxrCmdList->SetComputeRootShaderResourceView(1, a_execInfo.TlasBufferAddr);
		dxrCmdList->SetComputeRootConstantBufferView(2, a_execInfo.SceneCbvAddr);

		// #note_renderPass: A ray dispatch configures the shader table, consisting of shader records which identify how the GPU can find the resources
		// to invoke the attached shader. As the application currently only uses a global root signature these are sized to the shader identifier and aligned
		// to the SHADER_TABLE_BYTE_SIZE.
		const u64 ShaderTableGpuAddr = m_rtpso.ShaderTable->GetGPUVirtualAddress();
		const D3D12_DISPATCH_RAYS_DESC dispatchDesc =
		{
			.RayGenerationShaderRecord =
			{
				.StartAddress = ShaderTableGpuAddr,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
			},
			.MissShaderTable =
			{
				.StartAddress = ShaderTableGpuAddr + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
				.StrideInBytes = 0
			},
			.HitGroupTable =
			{
				.StartAddress = ShaderTableGpuAddr + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
				.StrideInBytes = 0
			},

			// These 3 values map to DispatchRaysDimensions.
			.Width = a_execInfo.SurfaceWidth,
			.Height = a_execInfo.SurfaceHeight,
			.Depth = 1
		};

		dxrCmdList->DispatchRays(&dispatchDesc);
	}

	void RenderPass::CreateRayTraceDemoRootSig()
	{
		CD3DX12_DESCRIPTOR_RANGE uavRange;
		uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		FixedArray<CD3DX12_ROOT_PARAMETER, 3> rootParams;
		rootParams[0].InitAsDescriptorTable(1, &uavRange);
		rootParams[1].InitAsShaderResourceView(0, 0);
		rootParams[2].InitAsConstantBufferView(0, 0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_0(static_cast<u32>(rootParams.size()), rootParams.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		//#Todo: Add proper error checking on the return blob.
		ComPtr<ID3DBlob> blob = nullptr;
		D3D12_CHECK(D3D12SerializeVersionedRootSignature(&rootSigDesc, &blob, nullptr));
		D3D12_CHECK(m_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_rootSig)));
		D3D12_NAME_OBJECT(m_rootSig, WString(L"RenderPass_RootSig"));
	}

	void RenderPass::CreateRayTracingPipelineStateObject()
	{
		const ShaderCompilationOptions options =
		{
			.ShaderModel = EShaderModel::SM6_3,
			.WarningsAreErrors = true,
			.SaveSymbols = true,
			.SaveReflection = true
		};

		//#Todo: Move the inline compilation to a different place once abstraction begins.
		ShaderCompilationOutput compileRes;
		if (!ShaderCompiler::CompileShaderFile(
			Path(ENGINE_SHADER_DIRECTORY) / "raytracer.rt.hlsl",
			Path(ENGINE_CACHE_DIRECTORY) / "shaders",
			options,
			compileRes))
		{
			return;
		}

		const FixedArray<D3D12_EXPORT_DESC, 3> shaderExports =
		{
			D3D12_EXPORT_DESC
			{
				.Name = L"LightPassRGS",
				.ExportToRename = L"RayGeneration",
				.Flags = D3D12_EXPORT_FLAG_NONE
			},
			D3D12_EXPORT_DESC
			{
				.Name = L"LightPassCHS",
				.ExportToRename = L"ClosestHit",
				.Flags = D3D12_EXPORT_FLAG_NONE
			},
			D3D12_EXPORT_DESC
			{
				.Name = L"LightPassMHS",
				.ExportToRename = L"Miss",
				.Flags = D3D12_EXPORT_FLAG_NONE
			}
		};

		const D3D12_DXIL_LIBRARY_DESC libDesc =
		{
			.DXILLibrary =
			{
				compileRes.Binary.Data,
				compileRes.Binary.SizeInBytes
			},
			.NumExports = static_cast<u32>(shaderExports.size()),
			.pExports = shaderExports.data()
		};

		const D3D12_HIT_GROUP_DESC hitGroupDesc =
		{
			.HitGroupExport = L"LightPassHG",
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.ClosestHitShaderImport = L"LightPassCHS"
		};

		const D3D12_RAYTRACING_SHADER_CONFIG shaderConfig =
		{
			.MaxPayloadSizeInBytes = 24,
			.MaxAttributeSizeInBytes = 8
		};

		// hmmmmm, do I need this? Global signature shows not necessarily, though I'm unsure :/
		//const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION hitgroupAssociation =
		//{
		//	.pSubobjectToAssociate = shaderConfig
		//};

		const D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig =
		{
			.MaxTraceRecursionDepth = 3
		};

		const D3D12_GLOBAL_ROOT_SIGNATURE globalSig =
		{
			.pGlobalRootSignature = m_rootSig.Get()
		};	

		const FixedArray<D3D12_STATE_SUBOBJECT, 5> subObjects =
		{
			D3D12_STATE_SUBOBJECT
			{
				.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, 
				.pDesc = &libDesc 
			},
			D3D12_STATE_SUBOBJECT
			{
				.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, 
				.pDesc = &hitGroupDesc 
			},
			D3D12_STATE_SUBOBJECT
			{
				.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, 
				.pDesc = &shaderConfig 
			},
			D3D12_STATE_SUBOBJECT
			{
				.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, 
				.pDesc = &globalSig 
			},
			D3D12_STATE_SUBOBJECT
			{
				.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, 
				.pDesc = &pipelineConfig 
			}
		};

		const D3D12_STATE_OBJECT_DESC rtpsoDesc =
		{
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<u32>(subObjects.size()),
			.pSubobjects = subObjects.data()
		};

		ComPtr<ID3D12Device5> dxrDevice;
		D3D12_CHECK(m_device.As(&dxrDevice));
		D3D12_CHECK(dxrDevice->CreateStateObject(&rtpsoDesc, IID_PPV_ARGS(&m_rtpso.Pso)));		
	}

	void RenderPass::CreateShaderTable()
	{
		// #note_renderpass: Currently hard coded once/if the shader file splits up in multiple files for each hit group this will not be hardcoded anymore.
		const u32 NumInternalShaders = 3; //raygen, closesthit and miss.
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(NumInternalShaders * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		D3D12_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_rtpso.ShaderTable)));
		if (!m_rtpso.ShaderTable)
		{
			DXRAY_ERROR("Failed to create rtpso shader ids buffer!");
			return;
		}

		ComPtr<ID3D12StateObjectProperties> psoProps = nullptr;
		D3D12_CHECK(m_rtpso.Pso.As(&psoProps));

		const FixedArray<void*, NumInternalShaders> shaderIds =
		{
			psoProps->GetShaderIdentifier(L"LightPassRGS"),
			psoProps->GetShaderIdentifier(L"LightPassMHS"),
			psoProps->GetShaderIdentifier(L"LightPassHG")
		};

		void* data = nullptr;
		D3D12_CHECK(m_rtpso.ShaderTable->Map(0, nullptr, &data));

		for (u32 i = 0; i < shaderIds.size(); i++)
		{
			memcpy(data, shaderIds[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			data = static_cast<u8*>(data) + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		}

		m_rtpso.ShaderTable->Unmap(0, nullptr);
	}
}
