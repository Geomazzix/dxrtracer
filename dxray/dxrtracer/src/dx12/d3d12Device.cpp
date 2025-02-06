#include "dxrtracer/dx12/d3d12Device.h"

namespace dxray
{
	D3D12Device::D3D12Device(const DeviceCreateInfo& a_info) :
		m_bUseWarp(a_info.bUseWarp)
	{
		u32 dxgiFactoryFlags = 0;
#ifndef CONFIG_RELEASE
		{
            //Debug layers.
			ComPtr<ID3D12Debug> debugController;
			D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			//For "Device Removed Extended Data".
			ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dredSettings;
			D3D12_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)));
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
#endif

		//Device creation.
		D3D12_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
		if (m_bUseWarp)
		{
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			D3D12_CHECK(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
			D3D12_CHECK(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
            D3D12_NAME_OBJECT(m_device, WString(L"D3D12WarpDevice"));
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
			//#Todo: Could possibly query for specific GPUs - if the wrong GPU is ever selected.
			D3D12_CHECK(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
            D3D12_NAME_OBJECT(m_device, WString(L"D3D12Device"));
		}

#ifndef CONFIG_RELEASE
		{
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			D3D12_CHECK(m_device.As(&pInfoQueue));

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
#endif

		m_presentQueue = std::make_unique<D3D12CommandQueue>(m_device, ECommandQueueType::Present);
		m_graphicsQueue = std::make_unique<D3D12CommandQueue>(m_device, ECommandQueueType::Graphics);
		m_computeQueue = std::make_unique<D3D12CommandQueue>(m_device, ECommandQueueType::Compute);
		m_copyQueue = std::make_unique<D3D12CommandQueue>(m_device, ECommandQueueType::Copy);

		CreateSwapchain(a_info.SwapchainInfo);
	}

	D3D12Device::~D3D12Device()
	{
		WaitIdle();

#ifndef CONFIG_RELEASE
		ComPtr<IDXGIDebug1> dxgiDebug;
		D3D12_CHECK(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiDebug.GetAddressOf())));
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
#endif
	}

	void D3D12Device::BeginFrame()
	{

	}

	void D3D12Device::EndFrame()
	{
		
	}

    void D3D12Device::Present()
    {
		D3D12_CHECK(m_swapchain->Present(1, 0));
		m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();
    }

    void D3D12Device::WaitIdle()
    {
		m_presentQueue->WaitIdle();
		m_graphicsQueue->WaitIdle();
		m_computeQueue->WaitIdle();
		m_copyQueue->WaitIdle();
    }

    void D3D12Device::CreateSwapchain(const SwapchainCreateInfo& a_swapchainInfo)
	{
		const DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
		{
			.Width = static_cast<u32>(a_swapchainInfo.SwapchainRect.Width),
			.Height = static_cast<u32>(a_swapchainInfo.SwapchainRect.Height),
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = { 1, 0 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = static_cast<u32>(m_renderTargets.size()),
			.Scaling = DXGI_SCALING_NONE, //#Note: Could be configurable...?
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0
		};

		ComPtr<IDXGISwapChain1> swapchain;
		D3D12_CHECK(m_factory->CreateSwapChainForHwnd(
			m_presentQueue->GetPresentQueue().Get(),
			static_cast<HWND>(a_swapchainInfo.WindowHandle),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapchain
		));

		//#Todo: full screen support.
		//Disable full screening for now - true full screen is often skipped and faked as border-less full-screen.
		D3D12_CHECK(m_factory->MakeWindowAssociation(static_cast<HWND>(a_swapchainInfo.WindowHandle), DXGI_MWA_NO_ALT_ENTER));
		D3D12_CHECK(swapchain.As(&m_swapchain));
		m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();

		//Create swap chain descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			const D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
			{
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = FrameCount,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0
			};
			D3D12_CHECK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		//Retrieve the swap chain render targets.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (u32 i = 0; i < FrameCount; i++)
			{
				D3D12_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
				m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}
	}

	std::shared_ptr<D3D12CommandBuffer> D3D12Device::RequestCommandBuffer(const ECommandBufferType a_type)
	{
		return GetQueue(static_cast<ECommandQueueType>(a_type))->RequestCommandBuffer();
	}

    u64 D3D12Device::ExecuteCommandLIst(const std::shared_ptr<D3D12CommandBuffer>& a_cmdBuffer)
    {
		return GetQueue(static_cast<ECommandQueueType>(a_cmdBuffer->GetType()))->ExecuteCommandList(a_cmdBuffer);
    }
}