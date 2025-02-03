#include "dxrtracer/dx12/d3d12Device.h"
#include <dxgidebug.h>

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
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
			//#Todo: Could possibly query for specific GPUs - if the wrong GPU is ever selected.
			D3D12_CHECK(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
		}

#ifndef CONFIG_RELEASE
		{
            //DXGI info queue.
            ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
            D3D12_CHECK(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())));
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			//Device info queue.
            ComPtr<ID3D12InfoQueue> pInfoQueue;
            D3D12_CHECK(m_device.As(&pInfoQueue));

			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

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

		//Create the command queue and allocator.
		const D3D12_COMMAND_QUEUE_DESC queueDesc =
		{
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
		};
		D3D12_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
		D3D12_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

		CreateSwapchain(a_info.SwapchainInfo);
	}

	D3D12Device::~D3D12Device()
	{

	}

	void D3D12Device::CreateSwapchain(const SwapchainCreateInfo& a_swapchainInfo)
	{
		const DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
		{
			.Width = static_cast<u32>(a_swapchainInfo.RenderTargetRect.Width),
			.Height = static_cast<u32>(a_swapchainInfo.RenderTargetRect.Height),
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

		ComPtr<IDXGISwapChain1> swapChain;
		D3D12_CHECK(m_factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			static_cast<HWND>(a_swapchainInfo.WindowHandle),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		//#Todo: full screen support.
		//Disable full screening for now - true fullscreen is often skipped and faked as borderless fullscreen.
		D3D12_CHECK(m_factory->MakeWindowAssociation(static_cast<HWND>(a_swapchainInfo.WindowHandle), DXGI_MWA_NO_ALT_ENTER));
		D3D12_CHECK(swapChain.As(&m_swapChain));
		m_swapchainIndex = m_swapChain->GetCurrentBackBufferIndex();

		//Create swapchain descriptor heaps.
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

		//Retrieve the swapchain render targets.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (u32 i = 0; i < FrameCount; i++)
			{
				D3D12_CHECK(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
				m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}
	}
}