#include "dxrtracer/renderer.h"
#include "dxrtracer/window.h"
#include "dxrtracer/renderpass.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/uploadBuffer.h"
#include "dxrtracer/camera.h"
#include <core/vath/vector4.h>

namespace dxray
{
	inline WString CommandListTypeToUnicode(const D3D12_COMMAND_LIST_TYPE a_type)
	{
		switch (a_type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
		{
			return L"Direct";
		}
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		{
			return L"Compute";
		}
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		{
			return L"Bundle";
		}
		case D3D12_COMMAND_LIST_TYPE_COPY:
		{
			return L"Copy";
		}
		default:
			return L"Unidentified";
		}

		return L"Unidentified";
	}


	CommandQueue::CommandQueue() :
		Handle(nullptr),
		Fence(nullptr),
		FenceEvent(0),
		FenceValue(0)
	{}

	CommandQueue::~CommandQueue()
	{
		WaitIdle();
		CloseHandle(FenceEvent);
	}

	u64 CommandQueue::Signal()
	{
		D3D12_CHECK(Handle->Signal(Fence.Get(), ++FenceValue));
		return FenceValue;
	}

	void CommandQueue::WaitForFence(const u64 a_fenceValue)
	{
		const u64 completedValue = Fence->GetCompletedValue();
		if (completedValue < a_fenceValue)
		{
			D3D12_CHECK(Fence->SetEventOnCompletion(a_fenceValue, FenceEvent));
			WaitForSingleObject(FenceEvent, u32max);
		}
	}

	void CommandQueue::WaitIdle()
	{
		WaitForFence(Signal());
	}


	Renderer::Renderer(const RendererCreateInfo& a_createInfo) :
		m_mainCamera(a_createInfo.MainCam),
		m_useWarp(false),
		m_swapchainIndex(0),
		m_sceneCbStride(CalculateConstantBufferSize(sizeof(SceneConstantBuffer)))
	{
		m_device = CreateDevice();
		m_graphicsQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
				
		CreateSwapchain(a_createInfo.SwapchainInfo);
		CreateFrameResources();

		m_renderPass = std::make_unique<RenderPass>(m_device);
	}

	Renderer::~Renderer()
	{
		m_cbvSceneHeap->Unmap(0, nullptr);
		m_graphicsQueue->WaitIdle();
	}

	void Renderer::Render(const fp32 a_dt)
	{
		FrameResources& frameResources = m_frameResources[m_swapchainIndex];
		m_graphicsQueue->WaitForFence(frameResources.FenceValue);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		D3D12_CHECK(m_swapchain->GetDesc1(&swapchainDesc));

		D3D12_CHECK(frameResources.CommandAllocator->Reset());
		D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));

		/* Update the scene constant buffer data. */
		frameResources.SceneConstantBufferData.View = vath::Inverse(m_mainCamera->GetViewMatrix());
		frameResources.SceneConstantBufferData.Projection = m_mainCamera->GetProjectionMatrix();
		frameResources.SceneConstantBufferData.FrameIndex = m_swapchainIndex;
		memcpy(reinterpret_cast<u8*>(m_cbvSceneHeapAddr) + m_sceneCbStride * m_swapchainIndex, &frameResources.SceneConstantBufferData, sizeof(SceneConstantBuffer));

		UpdateTlas(m_device, m_commandList, frameResources.WorldTlas, m_sceneObjectInstances);

		const RenderPassExecuteInfo executionInfo =
		{
			.BindlessHeap = m_bindlessHeap,
			.SceneCbvAddr = m_cbvSceneHeap->GetGPUVirtualAddress() + m_sceneCbStride * m_swapchainIndex,
			.TlasBufferAddr = frameResources.WorldTlas.Buffer->GetGPUVirtualAddress(),
			.RenderTargetUavSlot = 1024 + m_swapchainIndex,
			.SurfaceWidth = swapchainDesc.Width,
			.SurfaceHeight = swapchainDesc.Height
		};
		m_renderPass->Execute(m_commandList, executionInfo);
		
		Present(frameResources.RaytraceRenderTarget);

		D3D12_CHECK(m_commandList->Close());
		ID3D12CommandList* const lists[] = { m_commandList.Get() };
		m_graphicsQueue->Handle->ExecuteCommandLists(1, lists);

		D3D12_CHECK(m_swapchain->Present(0, 0));
		frameResources.FenceValue = m_graphicsQueue->Signal();
		m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();
	}

	void Renderer::Present(ComPtr<ID3D12Resource>& a_renderTargetOutput)
	{
		ComPtr<ID3D12Resource>& swapchainRenderTarget = m_swapchainRenderTargets[m_swapchainIndex];
		const std::array <CD3DX12_RESOURCE_BARRIER, 2> barriers =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(a_renderTargetOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(swapchainRenderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST)
		};
		m_commandList->ResourceBarrier(static_cast<u32>(barriers.size()), barriers.data());
		m_commandList->CopyResource(swapchainRenderTarget.Get(), a_renderTargetOutput.Get());

		const std::array <CD3DX12_RESOURCE_BARRIER, 2> presentationBarriers =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(swapchainRenderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT),
			CD3DX12_RESOURCE_BARRIER::Transition(a_renderTargetOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		};
		m_commandList->ResourceBarrier(static_cast<u32>(presentationBarriers.size()), presentationBarriers.data());
	}

	ComPtr<ID3D12Device> Renderer::CreateDevice()
	{
		u32 dxgiFactoryFlags = 0;
#ifndef CONFIG_RELEASE
		{
			//Debug layers.
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
			else
			{
				DXRAY_WARN("Could not retrieve DRED d3d12 debug interface, if this debugging layer is needed ensure that a compatible SDK and devtools are installed");
			}

			//For "Device Removed Extended Data".
			ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dredSettings;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings))))
			{
				dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			}
			else
			{
				DXRAY_WARN("Could not retrieve DRED d3d12 debug interface, if this debugging layer is needed ensure that a compatible SDK and devtools are installed");
			}
		}
#endif

		//Device creation.
		ComPtr<ID3D12Device> device;
		D3D12_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
		if (!m_useWarp)
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
			//#Todo: Could possibly query for specific GPUs - if the wrong GPU is ever selected.
			//       Ensure that the selected GPU supports the REQUIRED feature set, excluding potential optional features.
			D3D12_CHECK(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));
			if (device != nullptr)
			{
				D3D12_NAME_OBJECT(device, WString(L"D3D12Device"));
			}
		}

		if (device == nullptr || m_useWarp)
		{
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			D3D12_CHECK(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			D3D12_CHECK(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));
			D3D12_NAME_OBJECT(device, WString(L"D3D12WarpDevice"));
			DXRAY_CRITICAL("While warp is initializable - is does not support raytracing and should not be used!");
		}

#ifndef CONFIG_RELEASE
		{
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			if (SUCCEEDED(device.As(&pInfoQueue)))
			{
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

				// Suppress messages based on their severity level
				D3D12_MESSAGE_SEVERITY Severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};

				// Suppress individual messages by their ID
				D3D12_MESSAGE_ID DenyIds[] = {
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
				};

				D3D12_INFO_QUEUE_FILTER newFilter = {};
				newFilter.DenyList.NumSeverities = _countof(Severities);
				newFilter.DenyList.pSeverityList = Severities;
				newFilter.DenyList.NumIDs = _countof(DenyIds);
				newFilter.DenyList.pIDList = DenyIds;
				D3D12_CHECK(pInfoQueue->PushStorageFilter(&newFilter));
			}
		}
#endif

		return device;
	}

	std::unique_ptr<CommandQueue> Renderer::CreateCommandQueue(const D3D12_COMMAND_LIST_TYPE a_type)
	{
		const D3D12_COMMAND_QUEUE_DESC queueInfo =
		{
			.Type = a_type,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};

		std::unique_ptr<CommandQueue> commandQueue = std::make_unique<CommandQueue>();
		D3D12_CHECK(m_device->CreateCommandQueue(&queueInfo, IID_PPV_ARGS(&commandQueue->Handle)));
		D3D12_NAME_OBJECT(commandQueue->Handle, std::format(L"D3D12{}CommandQueue", CommandListTypeToUnicode(a_type)));

		D3D12_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&commandQueue->Fence)));
		D3D12_NAME_OBJECT(commandQueue->Fence, std::format(L"D3D12{}CommandQueueFence", CommandListTypeToUnicode(a_type)));

		commandQueue->FenceEvent = CreateEvent(nullptr, false, false, nullptr);
		DXRAY_ASSERT(commandQueue->FenceEvent);
		commandQueue->FenceValue = 0;

		return std::move(commandQueue);
	}

	void Renderer::CreateSwapchain(const SwapchainCreateInfo& a_swapchainInfo)
	{
		const DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
		{
			.Width = a_swapchainInfo.SurfaceWidth,
			.Height = a_swapchainInfo.SurfaceHeight,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = { 1, 0 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = static_cast<u32>(m_swapchainRenderTargets.size()),
			.Scaling = DXGI_SCALING_NONE, //#Note: Could be configurable...?
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0
		};

		ComPtr<IDXGISwapChain1> swapchain;
		D3D12_CHECK(m_factory->CreateSwapChainForHwnd(
			m_graphicsQueue->Handle.Get(),
			static_cast<HWND>(a_swapchainInfo.Window->GetNativeHandle()),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapchain
		));

		// #Todo: full screen support.
		// Disable full screening for now - true full screen is often skipped and faked as border-less full-screen.
		D3D12_CHECK(m_factory->MakeWindowAssociation(static_cast<HWND>(a_swapchainInfo.Window->GetNativeHandle()), DXGI_MWA_NO_ALT_ENTER));
		D3D12_CHECK(swapchain.As(&m_swapchain));
		m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();

		//Retrieve the swap chain render targets.
		m_rtvHeap = DescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapchainBackbufferCount, false);
		for (u32 i = 0; i < m_swapchainRenderTargets.size(); i++)
		{
			D3D12_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainRenderTargets[i])));
			m_device->CreateRenderTargetView(m_swapchainRenderTargets[i].Get(), nullptr, m_rtvHeap.GetCpuHandle(i));
		}
	}

	void Renderer::CreateFrameResources()
	{
		m_bindlessHeap = DescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1040, true);

		/* Scene constant buffer */
		const D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer(SwapchainBackbufferCount * m_sceneCbStride);

		D3D12_CHECK(m_device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &cbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_cbvSceneHeap)));
		D3D12_NAME_OBJECT(m_cbvSceneHeap, WString(L"CbvSceneHeap{}"));
		D3D12_CHECK(m_cbvSceneHeap->Map(0, nullptr, &m_cbvSceneHeapAddr)); // Kept mapped for the lifetime of the application, values in here are nearly 100% to change every frame.

		/* On frame basis data. */
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		D3D12_CHECK(m_swapchain->GetDesc1(&swapchainDesc));

		const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
			.Texture2D = 
			{
				.MipSlice = 0, 
				.PlaneSlice = 0 
			},
		};

		for (u32 i = 0; i < SwapchainBackbufferCount; i++)
		{
			FrameResources& frameResources = m_frameResources[i];
			frameResources.SceneConstantBufferData =
			{
				.SkyColour = vath::Vector4f(0.24f, 0.44f, 0.72f, 1.0),
				.SunDirection = vath::Vector4f(0.0f, 200.0f, 0.0f, 1.0f) // #Todo: Replace with direction - currently using position for the shadow ray description.
			};
			
			D3D12_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameResources.CommandAllocator)));
			D3D12_NAME_OBJECT(frameResources.CommandAllocator, std::format(L"D3D12{}CommandAllocator_{}", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT), std::to_wstring(i)));

			const CD3DX12_RESOURCE_DESC shaderRtDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, swapchainDesc.Width, swapchainDesc.Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			const CD3DX12_HEAP_PROPERTIES shaderRtHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			D3D12_CHECK(m_device->CreateCommittedResource(&shaderRtHeapProps, D3D12_HEAP_FLAG_NONE, &shaderRtDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&frameResources.RaytraceRenderTarget)));
			D3D12_NAME_OBJECT(frameResources.RaytraceRenderTarget, std::format(L"RaytraceRenderTarget_{}", i));

			m_device->CreateUnorderedAccessView(frameResources.RaytraceRenderTarget.Get(), nullptr, &uavDesc, m_bindlessHeap.GetCpuHandle(1024 + i));

			frameResources.FenceValue = i;
		}

		/* For simplicity's sake this application currently only uses 1 command list. */
		D3D12_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
		D3D12_NAME_OBJECT(m_commandList, std::format(L"D3D12{}CommandList", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT)));
		m_commandList->Close();
	}


	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// #note_renderer: Resource loading methods below are sort of a hack. These could be abstracted into a proper api if command list pooling were to be supported.

	void Renderer::BeginResourceLoading()
	{
		FrameResources& frameResources = m_frameResources[m_swapchainIndex];
		D3D12_CHECK(frameResources.CommandAllocator->Reset());
		D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));
	}

	void Renderer::LoadModel(const vath::Vector3f& a_location, const vath::Vector3f& a_eulerRotation, const vath::Vector3f& a_scale, const Model& a_model)
	{
		const auto WriteStructuredBufferSrv = [](ComPtr<ID3D12Device>& a_device, D3D12_CPU_DESCRIPTOR_HANDLE a_dest, ComPtr<ID3D12Resource>& a_resource, u32 a_numElements, u32 a_stride)
		{
			const D3D12_SHADER_RESOURCE_VIEW_DESC desc =
			{
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer =
				{
					.FirstElement = 0,
					.NumElements = a_numElements,
					.StructureByteStride = a_stride,
					.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
				},
			};

			a_device->CreateShaderResourceView(a_resource.Get(), &desc, a_dest);
		};

		for (const Mesh& mesh : a_model.Meshes)
		{
			/* Create the mesh itself */
			D3D12Mesh d3d12Mesh;
		
			d3d12Mesh.VertexCount = static_cast<u32>(mesh.Vertices.size());
			CreateReadBackBuffer(m_device, d3d12Mesh.VertexBuffer, mesh.Vertices.data(), mesh.Vertices.size() * sizeof(Vertex));
			D3D12_NAME_OBJECT(d3d12Mesh.VertexBuffer, std::format(L"{}{}", a_model.DebugName, L"_vertex_position_buffer"));

			d3d12Mesh.IndexCount = static_cast<u32>(mesh.Indices.size());
			CreateReadBackBuffer(m_device, d3d12Mesh.IndexBuffer, mesh.Indices.data(), mesh.Indices.size() * sizeof(u32));
			D3D12_NAME_OBJECT(d3d12Mesh.IndexBuffer, std::format(L"{}{}", a_model.DebugName, L"_index_buffer"));

			CreateBlas(m_device, m_commandList, d3d12Mesh.Blas, d3d12Mesh.VertexBuffer, mesh.Vertices.size(), d3d12Mesh.IndexBuffer, mesh.Indices.size());
			D3D12_NAME_OBJECT(d3d12Mesh.Blas.Scratch, std::format(L"{}{}", a_model.DebugName, L"_Blas_Scratch"));
			D3D12_NAME_OBJECT(d3d12Mesh.Blas.Buffer, std::format(L"{}{}", a_model.DebugName, L"_Blas"));
			m_meshes.push_back(d3d12Mesh);

			const u32 meshIdx = static_cast<u32>(m_meshes.size() - 1);

			/* Then assign the mesh indices to the bindless descriptor heap. */
			WriteStructuredBufferSrv(m_device, m_bindlessHeap.GetCpuHandle(1 + meshIdx), d3d12Mesh.VertexBuffer, d3d12Mesh.VertexCount, sizeof(Vertex));
			WriteStructuredBufferSrv(m_device, m_bindlessHeap.GetCpuHandle(257 + meshIdx), d3d12Mesh.IndexBuffer, d3d12Mesh.IndexCount, sizeof(u32));

			m_meshData.push_back(MeshData
			{
				.VertexBufferIdx = meshIdx,
				.IndexBufferIdx = meshIdx
			});

			/* Lastly specify the Tlas data, so the mesh can be instanced. */
			D3D12_RAYTRACING_INSTANCE_DESC desc =
			{
				.InstanceID = static_cast<u32>(m_meshes.size() - 1),
				.InstanceMask = 1,
				.InstanceContributionToHitGroupIndex = 0,
				.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE,
				.AccelerationStructure = d3d12Mesh.Blas.Buffer->GetGPUVirtualAddress()
			};

			// #Todo: Replace with Vath, get rid of all DirectX math.
			const DirectX::XMFLOAT3 euler(a_eulerRotation.x, a_eulerRotation.y, a_eulerRotation.z);
			DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(a_scale.x, a_scale.y, a_scale.z);
			transform *= DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&euler));
			transform *= DirectX::XMMatrixTranslation(a_location.x, a_location.y, a_location.z);
			XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(&desc.Transform), transform);
			m_sceneObjectInstances.push_back(desc);
		}
	}

	void Renderer::EndResourceLoading()
	{
		if (m_meshData.empty())
		{
			return;
		}

		const usize bytes = m_meshData.size() * sizeof(MeshData);
		CreateReadBackBuffer(m_device, m_meshDataBuffer, m_meshData.data(), bytes);
		D3D12_NAME_OBJECT(m_meshDataBuffer, std::format(L"{}", L"_MeshData_StructuredBuffer"));

		const D3D12_SHADER_RESOURCE_VIEW_DESC desc =
		{
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Buffer =
			{
				.FirstElement = 0,
				.NumElements = static_cast<u32>(m_meshData.size()),
				.StructureByteStride = sizeof(MeshData),
				.Flags = D3D12_BUFFER_SRV_FLAG_NONE,
			},
		};
		m_device->CreateShaderResourceView(m_meshDataBuffer.Get(), &desc, m_bindlessHeap.GetCpuHandle(0));

		D3D12_CHECK(m_commandList->Close());
		ID3D12CommandList* const lists[] = { m_commandList.Get() };
		m_graphicsQueue->Handle->ExecuteCommandLists(1, lists);
		m_graphicsQueue->WaitForFence(m_graphicsQueue->Signal());
	}

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
}