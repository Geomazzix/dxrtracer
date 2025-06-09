#include "dxrtracer/renderer.h"
#include "dxrtracer/window.h"
#include "dxrtracer/scene.h"
#include "dxrtracer/renderpass.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/uploadBuffer.h"

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
		WaitForFence(Fence->GetCompletedValue());
	}


	Renderer::Renderer(const RendererCreateInfo& a_createInfo) :
		m_useWarp(false),
		m_swapchainIndex(0)
	{
		m_graphicsQueue = std::make_unique<CommandQueue>();

		CreateDevice();
		CreateCommandQueue(*m_graphicsQueue.get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
		CreateSwapchain(a_createInfo.SwapchainInfo);
		CreateFrameResources();

		m_renderPass = std::make_unique<RenderPass>(m_device);
	}

	Renderer::~Renderer()
	{
		m_graphicsQueue->WaitIdle();
	}

	void Renderer::Render(std::shared_ptr<Scene>& a_pScene)
	{
		FrameResources& frameResources = m_frameResources[m_swapchainIndex];
		m_graphicsQueue->WaitForFence(frameResources.FenceValue);

		D3D12_CHECK(frameResources.CommandAllocator->Reset());
		D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));
		
		a_pScene->UpdateTlas(m_device, m_commandList, frameResources.WorldTlas, frameResources.WorldTlasInstancesData);
		
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		D3D12_CHECK(m_swapchain->GetDesc1(&swapchainDesc));
		
		const RenderPassExecuteInfo executionInfo = 
		{
			.UavHeap = m_uavHeap,
			.TlasBuffer = frameResources.WorldTlas.Buffer,
			.SwapchainIndex = m_swapchainIndex,
			.SurfaceWidth = swapchainDesc.Width,
			.SurfaceHeight = swapchainDesc.Height
		};
		m_renderPass->Execute(m_commandList, executionInfo);
		
		Present(frameResources.RaytraceRenderTarget);

		D3D12_CHECK(m_commandList->Close());
		ID3D12CommandList* const lists[] = { m_commandList.Get() };
		m_graphicsQueue->Handle->ExecuteCommandLists(1, lists);

		D3D12_CHECK(m_swapchain->Present(1, 0));
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

	void Renderer::CreateDevice()
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
		D3D12_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
		if (!m_useWarp)
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
			//#Todo: Could possibly query for specific GPUs - if the wrong GPU is ever selected.
			//       Ensure that the selected GPU supports the REQUIRED feature set, excluding potential optional features.
			D3D12_CHECK(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
			if (m_device != nullptr)
			{
				D3D12_NAME_OBJECT(m_device, WString(L"D3D12Device"));
			}
		}

		if (m_device == nullptr || m_useWarp)
		{
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			D3D12_CHECK(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			D3D12_CHECK(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
			D3D12_NAME_OBJECT(m_device, WString(L"D3D12WarpDevice"));
			DXRAY_CRITICAL("While warp is initializable - is does not support raytracing and should not be used!");
		}

#ifndef CONFIG_RELEASE
		{
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			if (SUCCEEDED(m_device.As(&pInfoQueue)))
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
	}

	void Renderer::CreateCommandQueue(CommandQueue& a_commandQueue, const D3D12_COMMAND_LIST_TYPE a_type)
	{
		const D3D12_COMMAND_QUEUE_DESC queueInfo =
		{
			.Type = a_type,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};

		D3D12_CHECK(m_device->CreateCommandQueue(&queueInfo, IID_PPV_ARGS(&a_commandQueue.Handle)));
		D3D12_NAME_OBJECT(a_commandQueue.Handle, std::format(L"D3D12{}CommandQueue", CommandListTypeToUnicode(a_type)));

		D3D12_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&a_commandQueue.Fence)));
		D3D12_NAME_OBJECT(a_commandQueue.Fence, std::format(L"D3D12{}CommandQueueFence", CommandListTypeToUnicode(a_type)));

		a_commandQueue.FenceEvent = CreateEvent(nullptr, false, false, nullptr);
		DXRAY_ASSERT(a_commandQueue.FenceEvent);
		a_commandQueue.FenceValue = 0;
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

		//Create swap chain descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			const D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
			{
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = SwapchainBackbufferCount,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0
			};
			D3D12_CHECK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		//Retrieve the swap chain render targets.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (u32 i = 0; i < m_swapchainRenderTargets.size(); i++)
			{
				D3D12_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainRenderTargets[i])));
				m_device->CreateRenderTargetView(m_swapchainRenderTargets[i].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}
	}

	void Renderer::CreateFrameResources()
	{
		//#Todo: Check if this needs to be a frame dependent resource as well, seems odd to leave this here.
		const D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = SwapchainBackbufferCount,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			.NodeMask = 0
		};

		D3D12_CHECK(m_device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&m_uavHeap)));
		D3D12_NAME_OBJECT(m_uavHeap, std::format(L"UavDescriptorHeap"));

		m_uavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(m_uavHeap->GetCPUDescriptorHandleForHeapStart());

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
		D3D12_CHECK(m_swapchain->GetDesc1(&swapchainDesc));

		for (u32 i = 0; i < SwapchainBackbufferCount; i++)
		{
			FrameResources& frameResources = m_frameResources[i];
			D3D12_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameResources.CommandAllocator)));
			D3D12_NAME_OBJECT(frameResources.CommandAllocator, std::format(L"D3D12{}CommandAllocator_{}", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT), std::to_wstring(i)));

			const D3D12_RESOURCE_DESC shaderRenderTarget =
			{
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Width = swapchainDesc.Width,
				.Height = swapchainDesc.Height,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc = { 1, 0 },
				.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			};

			const D3D12_HEAP_PROPERTIES heapDesc =
			{
				.Type = D3D12_HEAP_TYPE_DEFAULT
			};

			D3D12_CHECK(m_device->CreateCommittedResource(&heapDesc, D3D12_HEAP_FLAG_NONE, &shaderRenderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&frameResources.RaytraceRenderTarget)));
			D3D12_NAME_OBJECT(frameResources.RaytraceRenderTarget, std::format(L"RaytraceRenderTarget_{}", i));

			const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
			{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
				.Texture2D =
				{
					.MipSlice = 0,
					.PlaneSlice = 0
				}
			};
			m_device->CreateUnorderedAccessView(frameResources.RaytraceRenderTarget.Get(), nullptr, &uavDesc, uavHandle);
			uavHandle.Offset(1, m_uavDescriptorSize);

			frameResources.FenceValue = i;
		}

		D3D12_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
		D3D12_NAME_OBJECT(m_commandList, std::format(L"D3D12{}CommandList", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT)));
		m_commandList->Close();
	}


	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	// #note_renderer: Resource loading methods below are sort of a hack. These could be abstracted into a proper api if command list pooling were to be supported.

	void Renderer::CreateModelResources(ComPtr<ID3D12GraphicsCommandList>& a_cmdList, const Model& a_model)
	{
		for (const Mesh& mesh : a_model.Meshes)
		{
			SceneObjectRenderData sceneObjectRenderData;
			CreateReadBackBuffer(m_device, sceneObjectRenderData.VertexBuffer, mesh.Vertices.data(), mesh.Vertices.size() * Vertex::Stride);
			D3D12_NAME_OBJECT(sceneObjectRenderData.VertexBuffer, std::format(L"{}{}", a_model.DebugName, L"_vertex_buffer"));
			CreateReadBackBuffer(m_device, sceneObjectRenderData.IndexBuffer, mesh.Indices.data(), mesh.Indices.size() * sizeof(u32));
			D3D12_NAME_OBJECT(sceneObjectRenderData.IndexBuffer, std::format(L"{}{}", a_model.DebugName, L"_index_buffer"));

			CreateBlas(m_device, a_cmdList, sceneObjectRenderData.Blas, sceneObjectRenderData.VertexBuffer, mesh.Vertices.size(), sceneObjectRenderData.IndexBuffer, mesh.Indices.size());
			D3D12_NAME_OBJECT(sceneObjectRenderData.Blas.Scratch, std::format(L"{}{}", a_model.DebugName, L"_Blas_Scratch"));
			D3D12_NAME_OBJECT(sceneObjectRenderData.Blas.Buffer, std::format(L"{}{}", a_model.DebugName, L"_Blas"));
			m_sceneObjectRenderDataBuffer.push_back(sceneObjectRenderData);
		}
	}

	void Renderer::BeginResourceLoading()
	{
		FrameResources& frameResources = m_frameResources[m_swapchainIndex];
		D3D12_CHECK(frameResources.CommandAllocator->Reset());
		D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));
	}

	void Renderer::LoadModel(std::shared_ptr<Scene>& a_pScene, Model& a_model)
	{
		CreateModelResources(m_commandList, a_model);
	}

	void Renderer::EndResourceLoading(std::shared_ptr<Scene>& a_pScene)
	{
		for (usize blasIdx = 0; blasIdx < m_sceneObjectRenderDataBuffer.size(); blasIdx++)
		{
			a_pScene->AddSceneObjectInstance(m_sceneObjectRenderDataBuffer[blasIdx].Blas);
		}

		D3D12_CHECK(m_commandList->Close());
		ID3D12CommandList* const lists[] = { m_commandList.Get() };
		m_graphicsQueue->Handle->ExecuteCommandLists(1, lists);
		m_graphicsQueue->WaitForFence(m_graphicsQueue->Signal());
	}

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
}