#include "dxrtracer/dxShaderCompiler.h"
#include "dxrtracer/window.h"
#include <core/time/stopwatch.h>

//#Todo: no functions have proper error checking, add errors for different parts of the codebase.

// --- DX12 primitives ---

using namespace dxray;

Stopwatchf m_time;
std::unique_ptr<WinApiWindow> m_window;
std::unique_ptr<DxShaderCompiler> m_dxShaderCompiler;

bool m_bUseWarp;
ComPtr<IDXGIFactory4> m_factory;
ComPtr<ID3D12Device> m_device;

HANDLE m_commandQueueFenceEvent;
u64 m_commandQueueFenceValue;
ComPtr<ID3D12Fence> m_commandQueueFence;
ComPtr<ID3D12CommandQueue> m_commandQueue;
ComPtr<ID3D12GraphicsCommandList> m_commandList;

struct FrameResources
{
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	u64 FenceValue;
};

inline constexpr u32 SwapchainBackbufferCount = 2;
std::array<ComPtr<ID3D12Resource>, SwapchainBackbufferCount> m_swapchainRenderTargets;
std::array<FrameResources, SwapchainBackbufferCount> m_frameResources;
u32 m_swapchainIndex = 0;
u32 m_rtvDescriptorSize = 0;
ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

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


// --- Creating primitives ---

void CreateDevice(ComPtr<ID3D12Device>& a_device)
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
    if (!m_bUseWarp)
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
    
    if (m_device == nullptr || m_bUseWarp)
    {
		ComPtr<IDXGIAdapter> warpAdapter = nullptr;
		D3D12_CHECK(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		D3D12_CHECK(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
		D3D12_NAME_OBJECT(m_device, WString(L"D3D12WarpDevice"));
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

void CreateCommandQueue(ComPtr<ID3D12CommandQueue>& a_queue, const D3D12_COMMAND_LIST_TYPE a_type)
{
    const D3D12_COMMAND_QUEUE_DESC queueInfo =
    {
        .Type = a_type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };

    D3D12_CHECK(m_device->CreateCommandQueue(&queueInfo, IID_PPV_ARGS(&a_queue)));
    D3D12_NAME_OBJECT(m_commandQueue, std::format(L"D3D12{}CommandQueue", CommandListTypeToUnicode(a_type)));

    D3D12_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_commandQueueFence)));
    D3D12_NAME_OBJECT(m_commandQueueFence, std::format(L"D3D12{}CommandQueueFence", CommandListTypeToUnicode(a_type)));

    m_commandQueueFenceEvent = CreateEvent(nullptr, false, false, nullptr);
    DXRAY_ASSERT(m_commandQueueFenceEvent);
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
        .BufferCount = static_cast<u32>(m_swapchainRenderTargets.size()),
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
            .NumDescriptors = static_cast<u32>(m_swapchainRenderTargets.size()),
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

void CreateFrameResources()
{
    for (u32 i = 0; i < SwapchainBackbufferCount; i++)
    {
        FrameResources& frameResources = m_frameResources[i];
        D3D12_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameResources.CommandAllocator)));
		D3D12_NAME_OBJECT(frameResources.CommandAllocator, std::format(L"D3D12{}CommandAllocator_{}", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT), std::to_wstring(i)));
        frameResources.FenceValue = i;
    }

	D3D12_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameResources[0].CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	D3D12_NAME_OBJECT(m_commandList, std::format(L"D3D12{}CommandList", CommandListTypeToUnicode(D3D12_COMMAND_LIST_TYPE_DIRECT)));
    m_commandList->Close();
}

void WaitForCommandQueueFence(const u64 a_fenceValue)
{
    const u64 completedValue = m_commandQueueFence->GetCompletedValue();
    if (completedValue < a_fenceValue)
    {
		D3D12_CHECK(m_commandQueueFence->SetEventOnCompletion(a_fenceValue, m_commandQueueFenceEvent));
		WaitForSingleObject(m_commandQueueFenceEvent, u32max);
    }
}


// --- Engine loop ---

void Tick()
{

}

void Render()
{
    // -- Retrieve the data needed to render --
    FrameResources& frameResources = m_frameResources[m_swapchainIndex];
    WaitForCommandQueueFence(frameResources.FenceValue);

	// -- Record the data --
	D3D12_CHECK(frameResources.CommandAllocator->Reset());
    D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));

    ID3D12Resource* rt = m_swapchainRenderTargets[m_swapchainIndex].Get();
    CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainRenderTargets[m_swapchainIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &bar);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_swapchainIndex, m_rtvDescriptorSize);
	const fp32 clearColor[] = { 200/255.0f, 96.0f/255.f, 24.0f/255.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    CD3DX12_RESOURCE_BARRIER bar2 = CD3DX12_RESOURCE_BARRIER::Transition(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &bar2);

    D3D12_CHECK(m_commandList->Close());

    // -- Execute the data --
    ID3D12CommandList* const lists[] =
    {
        m_commandList.Get()
    };
    m_commandQueue->ExecuteCommandLists(1, lists);

    // -- End of render frame will present to the screen and signal the command queue --
    D3D12_CHECK(m_swapchain->Present(1, 0));
	D3D12_CHECK(m_commandQueue->Signal(m_commandQueueFence.Get(), ++m_commandQueueFenceValue));
    frameResources.FenceValue = m_commandQueueFenceValue;
	m_swapchainIndex = m_swapchain->GetCurrentBackBufferIndex();
}

void FrameLoop()
{
	fp32 elapsedInterval = 0.0f;
	u64 fps = 0u;

	while (m_window->PollEvents())
	{
        Tick();
        Render();

		++fps;
		if (m_time.GetElapsedSeconds() - elapsedInterval > 1.0f)
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


// --- Entrypoint ---

int main(int argc, char** argv)
{
    m_time.Start();
    const WindowCreationInfo windowInfo =
    {
        .Title = PROJECT_NAME,
        .Rect = dxray::vath::Rectu32(0, 0, 1600, 900)
    };
    m_window = std::make_unique<WinApiWindow>(windowInfo);
    m_dxShaderCompiler = std::make_unique<DxShaderCompiler>();
    
    const ShaderCompilationOptions options =
    {
		.OptimizeLevel = EOptimizeLevel::O2,
		.ShaderModel = EShaderModel::SM6_3,
		.ShouldKeepDebugInfo = true
    };

    //Currently compiles and saves binary data in intermediate directory, these will later be loaded back into memory.
    //Something that could potentially be done is allowing this dependency to be a standalone tool, though that's something for later.
    m_dxShaderCompiler->CompileShadersInDirectory(ENGINE_SHADER_DIRECTORY, Path(ENGINE_CACHE_DIRECTORY) / "shaders", options);

    CreateDevice(m_device);
    CreateCommandQueue(m_commandQueue, D3D12_COMMAND_LIST_TYPE_DIRECT);
    CreateSwapchain(m_swapchain, windowInfo.Rect.Width, windowInfo.Rect.Height);
    CreateFrameResources();

    FrameLoop();

	WaitForCommandQueueFence(m_commandQueueFence->GetCompletedValue()); //Flush the gpu.
	CloseHandle(m_commandQueueFenceEvent);

	return 0;
}