#include "dxrtracer/renderpass.h"
#include "dxrtracer/shaderCompiler.h"

namespace dxray
{
	RenderPass::RenderPass(ComPtr<ID3D12Device>& a_device) :
		m_device(a_device)
	{
		CreateRayTraceDemoRootSig();
		CreateRayTracingPipelineStateObject();
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
		dxrCmdList->SetComputeRootShaderResourceView(1, a_execInfo.TlasBuffer->GetGPUVirtualAddress());

		D3D12_DISPATCH_RAYS_DESC dispatchDesc =
		{
			.RayGenerationShaderRecord =
			{
				.StartAddress = m_rtpso.ShaderIds->GetGPUVirtualAddress(),
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
			},
			.MissShaderTable =
			{
				.StartAddress = m_rtpso.ShaderIds->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
			},
			.HitGroupTable =
			{
				.StartAddress = m_rtpso.ShaderIds->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
				.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
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
		const D3D12_DESCRIPTOR_RANGE uavRange =
		{
			.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			.NumDescriptors = 1,
		};

		std::array<D3D12_ROOT_PARAMETER, 2> params;

		//Render target binding.
		params[0] =
		{
			.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			.DescriptorTable =
			{
				.NumDescriptorRanges = 1,
				.pDescriptorRanges = &uavRange
			}
		};

		//TLas binding.
		params[1] =
		{
			.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
			.Descriptor =
			{
				.ShaderRegister = 0,
				.RegisterSpace = 0
			}
		};

		const D3D12_ROOT_SIGNATURE_DESC desc =
		{
			.NumParameters = static_cast<u32>(params.size()),
			.pParameters = params.data(),
			.NumStaticSamplers = 0,
			.pStaticSamplers = nullptr,
			.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
		};

		//#Todo: Add proper error checking on the return blob.
		ComPtr<ID3DBlob> blob = nullptr;
		D3D12_CHECK(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr));
		D3D12_CHECK(m_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_rootSig)));
	}

	void RenderPass::CreateRayTracingPipelineStateObject()
	{
		//1. Compile the shader and create a sub object for it.
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

		const D3D12_DXIL_LIBRARY_DESC libDesc =
		{
			.DXILLibrary =
			{
				compileRes.Binary.Data,
				compileRes.Binary.SizeInBytes
			},
			.NumExports = 0,
			.pExports = nullptr
		};

		//2. Define a hit group, which the dispatched rays refer to when looking for intersections. These include any form of hit shaders.
		const D3D12_HIT_GROUP_DESC hitGroupDesc =
		{
			.HitGroupExport = L"HitGroup",
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.ClosestHitShaderImport = L"ClosestHit"
		};

		//3. The shader config is responsible for matching the ray payload, defined in the file - #Todo: Investigate shader reflection possabilities.
		const D3D12_RAYTRACING_SHADER_CONFIG shaderConfig =
		{
			.MaxPayloadSizeInBytes = 20,
			.MaxAttributeSizeInBytes = 8
		};

		//4. Define a global root signature - #Todo: Investigate local root signatures https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#local-root-signatures-vs-global-root-signatures
		const D3D12_GLOBAL_ROOT_SIGNATURE globalSig =
		{
			m_rootSig.Get()
		};

		//5. Define the max bounds configuration, going above this will result in a driver crash.
		const D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig =
		{
			.MaxTraceRecursionDepth = 3
		};

		//6. Pack the subobjects into an array and create the rtpso.
		std::array<D3D12_STATE_SUBOBJECT, 5> subObjects;
		subObjects[0] = { .Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &libDesc };
		subObjects[1] = { .Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, .pDesc = &hitGroupDesc };
		subObjects[2] = { .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, .pDesc = &shaderConfig };
		subObjects[3] = { .Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, .pDesc = &globalSig };
		subObjects[4] = { .Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, .pDesc = &pipelineConfig };

		const D3D12_STATE_OBJECT_DESC rtpsoDesc =
		{
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<u32>(subObjects.size()),
			.pSubobjects = subObjects.data()
		};

		ComPtr<ID3D12Device5> dxrDevice;
		D3D12_CHECK(m_device.As(&dxrDevice));
		D3D12_CHECK(dxrDevice->CreateStateObject(&rtpsoDesc, IID_PPV_ARGS(&m_rtpso.Pso)));

		//7. Store the shader Ids so they can be identified when the rays have to be dispatched.
		const D3D12_RESOURCE_DESC resourceDesc =
		{
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Alignment = 0,
			.Width = m_rtpso.NumShaderIds * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, //#Note: shader ids are 32-byte, the other 32 byte required for alignment can be used for root params.
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc = { 1, 0 },
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_NONE
		};

		const D3D12_HEAP_PROPERTIES heapProps =
		{
			.Type = D3D12_HEAP_TYPE_UPLOAD
		};

		D3D12_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_rtpso.ShaderIds)));
		if (!m_rtpso.ShaderIds)
		{
			DXRAY_ERROR("Failed to create rtpso shader ids buffer!");
			return;
		}

		ComPtr<ID3D12StateObjectProperties> psoProps = nullptr;
		D3D12_CHECK(m_rtpso.Pso.As(&psoProps));

		void* data = nullptr;
		m_rtpso.ShaderIds->Map(0, nullptr, &data);

		//#Note: Size doesn't matter here as the shaderids are hardcoded anyways :/
		const std::vector<void*> shaderIds =
		{
			psoProps->GetShaderIdentifier(L"RayGeneration"),
			psoProps->GetShaderIdentifier(L"Miss"),
			psoProps->GetShaderIdentifier(L"HitGroup")
		};

		for (u32 i = 0; i < m_rtpso.NumShaderIds; i++)
		{
			memcpy(data, shaderIds[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			data = static_cast<u8*>(data) + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
		}

		m_rtpso.ShaderIds->Unmap(0, nullptr);
	}
}
