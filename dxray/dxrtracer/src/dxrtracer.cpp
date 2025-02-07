#include "dxrtracer/dxrtracer.h"
#include "dxrtracer/window.h"
#include "dxrtracer/renderer.h"
#include <core/time/stopwatch.h>

namespace dxray
{
	Stopwatchf DxrTracer::DxrTracer::s_appTime;

	DxrTracer::DxrTracer(const ApplicationCreateInfo& a_info)
	{
        s_appTime.Start();

		const WindowCreationInfo windowInfo =
		{
			.Title = a_info.Title,
			.Rect = a_info.Rect
		};

		m_window = std::make_unique<WinApiWindow>(windowInfo);

		const RendererCreateInfo rendererInfo =
		{
			.WindowRect = vath::Rectu32(0u, 0u, 1600u, 900u),
			.WindowHandle = m_window->GetNativeHandle(),
			.bUseWarp = false
		};
		
		m_renderer = std::make_unique<Renderer>(rendererInfo);

		EngineLoop();
	}

	void DxrTracer::EngineLoop()
	{
		fp32 elapsedInterval = 0.0f;
		u64 fps = 0u;

		while (m_window->PollEvents())
		{
			//m_renderer->Render();

			++fps;
			if (s_appTime.GetElapsedSeconds() - elapsedInterval > 1.0f)
			{
				m_window->SetWindowTitle(std::format("{} fps: {} - mspf: {}", 
					PROJECT_NAME, 
					static_cast<fp32>(fps), 
					static_cast<fp32>(1000.0f / fps))
				);

				elapsedInterval += 1.0f;
				fps = 0;
			}
		}
	}
}


using namespace dxray;









static const u32 SwapchainBackbufferCount = 2;


struct FrameResources
{
    
};
std::vector<FrameResources, SwapchainBackbufferCount> m_frameResources;

std::unique_ptr<WinApiWindow> m_window;

bool m_bUseWarp;
ComPtr<IDXGIFactory4> m_factory;
ComPtr<ID3D12Device> m_device;

u32 m_rtvDescriptorSize;
u32 m_swapchainIndex;
ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
ComPtr<IDXGISwapChain3> m_swapchain;
std::array<ComPtr<ID3D12Resource>, 2> m_swapchainBackBuffer;

ComPtr<ID3D12CommandQueue> m_commandQueue;
ComPtr<ID3D12CommandAllocator> m_commandAllocator;
ComPtr<ID3D12GraphicsCommandList> m_commandList;

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

void CreateDevice(ComPtr<ID3D12Device>& a_device)
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
}

void CreateCommandQueue(ComPtr<ID3D12CommandQueue>& a_queue, const D3D12_COMMAND_LIST_TYPE a_type)
{
    const D3D12_COMMAND_QUEUE_DESC queueInfo =
    {
        .Type = a_type,
        .Priority = 0,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };

    D3D12_CHECK(m_device->CreateCommandQueue(&queueInfo, IID_PPV_ARGS(&a_queue)));
    D3D12_NAME_OBJECT(m_commandQueue, std::format(L"D3D12{}CommandQueue", CommandListTypeToUnicode(a_type)));
}

void CreateSwapchain(ComPtr<IDXGISwapChain3>& a_swapchain, const u32 a_width, const u32 a_height)
{
    const DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
    {
        .Width = static_cast<u32>(a_width),
        .Height = static_cast<u32>(a_height),
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc = { 1, 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = static_cast<u32>(m_swapchainBackBuffer.size()),
        .Scaling = DXGI_SCALING_NONE, //#Note: Could be configurable...?
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = 0
    };

    ComPtr<IDXGISwapChain1> swapchain;
    D3D12_CHECK(m_factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        static_cast<HWND>(m_window->GetNativeHandle()),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapchain
    ));

    //#Todo: full screen support.
    //Disable full screening for now - true full screen is often skipped and faked as border-less full-screen.
    D3D12_CHECK(m_factory->MakeWindowAssociation(static_cast<HWND>(m_window->GetNativeHandle()), DXGI_MWA_NO_ALT_ENTER));
    D3D12_CHECK(swapchain.As(&m_swapchain));
    m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();

    //Create swap chain descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        const D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
        {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = static_cast<u32>(m_swapchainBackBuffer.size()),
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };
        D3D12_CHECK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    //Retrieve the swap chain render targets.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (u32 i = 0; i < m_swapchainBackBuffer.size(); i++)
        {
            D3D12_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainBackBuffer[i])));
            m_device->CreateRenderTargetView(m_swapchainBackBuffer[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }
}



int main(int argc, char** argv)
{
	const dxray::ApplicationCreateInfo appInfo =
	{
		.Title = PROJECT_NAME,
		.Rect = dxray::vath::Rectu32(0, 0, 1600, 900)
	};
	//dxray::DxrTracer m_application(appInfo);

    const WindowCreationInfo windowInfo =
    {
        .Title = appInfo.Title,
        .Rect = appInfo.Rect
    };

    m_window = std::make_unique<WinApiWindow>(windowInfo);
    
    CreateDevice(m_device);
    CreateSwapchain(m_swapchain, appInfo.Rect.Width, appInfo.Rect.Height);


	return 0;
}