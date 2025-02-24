#include "dxrtracer/dxShaderCompiler.h"
#include "dxrtracer/window.h"
#include <core/time/stopwatch.h>

using namespace dxray;

//#Todo: no functions have proper error checking, add errors for different parts of the codebase.
//#Todo: Implement something alike vk::ArrayProxy. This would serve as a wrapper for std::array, std::vector and std::initializer_list which prevents raw pointer arguments.

/*#Todo: Abstract acceleration structures into a scalable world hierarchy:
World (geometry data):
- Tlas (main all-owning Tlas)
    - Tlas (scene/chunk tlas - depending on whether world partitioning is a thing and whether depth based updating is a thing - see Cascaded Shadow Map update rates.)
        - Blas (x-amount per Tlas *should* be reusable. Note that the way these are constructed is heavily dependant on the type of geometry, see D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS.)

- Each Blas should be assigned to a mesh/model (or multiple, depending on the level of abstraction/optimization).
- Each Tlas should be owned by the world and managed by the world, update on the CPU, rebuild on the GPU.
- Each mesh/model has indices/handles that can be used to identify and set the PSO/rootsigs accordingly.
*/

//#Todo: When abstracting the CreateBlas into something class/data oriented ensure less arguments, this sucks :(


/*
!The last bits to get this to work:
1. Rewrite swapchain render target management -> this needs to use UAVs as the shader most likely won't allow non-UAV writing - raytracing shaders are considered compute shaders as far as I'm aware.
2. Implement the render function correctly.
*/


// --- Asset data ---

constexpr fp32 m_quadVertices[] = { 
    -1, 0, -1, 
    -1, 0,  1, 
    1, 0, 1,
    -1, 0, -1,  
    1, 0, -1, 
    1, 0, 1 
};

constexpr fp32 m_cubeVertices[] = {
    -1, -1, -1, 
    1, -1, -1, 
    -1, 1, -1, 
    1, 1, -1,
    -1, -1,  1, 
    1, -1,  1, 
    -1, 1,  1, 
    1, 1,  1 
};

constexpr u16 m_cubeIndicies[] = {
    4, 6, 0, 
    2, 0, 6, 
    0, 1, 4, 
    5, 4, 1,
    0, 2, 1, 
    3, 1, 2, 
    1, 3, 5, 
    7, 5, 3,
    2, 6, 3, 
    7, 3, 6, 
    4, 5, 6, 
    7, 6, 5 
};

ComPtr<ID3D12Resource> m_quadVertexBuffer;
ComPtr<ID3D12Resource> m_cubeVertexBuffer;
ComPtr<ID3D12Resource> m_cubeIndexBuffer;

struct AccelerationStructure
{
	ComPtr<ID3D12Resource> Buffer;
	ComPtr<ID3D12Resource> Scratch; //#Note: scratch memory is used during the building of the BVH. After this is can be reused for other purposes - source: do and don't from nvidia.
};

AccelerationStructure m_quadBlas;
AccelerationStructure m_cubeBlas;

u32 m_numBlas = 3; //3 objects, a cube, a mirror quad and the floor quad.
ComPtr<ID3D12Resource> m_blasInstanceBuffer;
void* m_blasInstanceBufferAddr;

ComPtr<ID3D12RootSignature> m_rootSig;

struct RaytracePipelineStateObject
{
    ComPtr<ID3D12StateObject> Pso;
    ComPtr<ID3D12Resource> ShaderIds;
    const u32 NumShaderIds = 3;
};
RaytracePipelineStateObject m_rtpso;


// --- DX12 primitives ---

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
	AccelerationStructure WorldTlas;
	ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
	u64 FenceValue = 0;
};

inline constexpr u32 SwapchainBackbufferCount = 3;
std::array<ComPtr<ID3D12Resource>, SwapchainBackbufferCount> m_swapchainRenderTargets;
std::array<FrameResources, SwapchainBackbufferCount> m_frameResources;
u32 m_swapchainIndex = 0;
u32 m_rtvDescriptorSize = 0;
ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

std::vector<ComPtr<ID3D12Resource>> m_bottomLevelAccelerationStructures;
ComPtr<ID3D12Resource> m_topLevelAccelerationStructure = nullptr;

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


// --- Creating D3D12 primitives ---

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

void CreateReadBackBuffer(ComPtr<ID3D12Resource>& a_resource, const void* a_pData, const u32 a_sizeInBytes)
{
    //#Todo: Horrendous non-optimized buffer, this needs proper upload management.

    //A buffer in d3d12 is represented as a 1 dimensional aligned resource.
    const D3D12_RESOURCE_DESC bufferDesc =
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
        .Flags = D3D12_RESOURCE_FLAG_NONE
    };

    const D3D12_HEAP_PROPERTIES uploadHeapProperties =
    {
		.Type = D3D12_HEAP_TYPE_UPLOAD
    };

    D3D12_CHECK(m_device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&a_resource)));
    if (a_resource != nullptr)
    {
		void* mappedAddr = nullptr;
        a_resource->Map(0, nullptr, &mappedAddr);
		memcpy(mappedAddr, a_pData, a_sizeInBytes);
        a_resource->Unmap(0, nullptr);
    }
}

void CreateSceneInstances()
{
	const D3D12_RESOURCE_DESC bufferDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_numBlas,
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

	D3D12_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_blasInstanceBuffer)));
	if (!m_blasInstanceBuffer)
	{
        return;
	}

	m_blasInstanceBuffer->Map(0, nullptr, &m_blasInstanceBufferAddr);
	D3D12_RAYTRACING_INSTANCE_DESC* const instances = static_cast<D3D12_RAYTRACING_INSTANCE_DESC* const>(m_blasInstanceBufferAddr);
	instances[0] =
	{
		.InstanceID = 0,
		.InstanceMask = 1,
		.AccelerationStructure = m_cubeBlas.Buffer->GetGPUVirtualAddress()
	};

	instances[1] =
	{
		.InstanceID = 1,
		.InstanceMask = 1,
		.AccelerationStructure = m_quadBlas.Buffer->GetGPUVirtualAddress()
	};

	instances[2] =
	{
		.InstanceID = 2,
		.InstanceMask = 1,
		.AccelerationStructure = m_quadBlas.Buffer->GetGPUVirtualAddress()
	};
}

void CreateAccelerationStructure(ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& a_asInputs)
{
	auto CreateGpuBuffer = [](u64 a_sizeInBytes, D3D12_RESOURCE_STATES a_initialState)
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
		D3D12_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, a_initialState, nullptr, IID_PPV_ARGS(&resource)));
		return resource;
	};

    ComPtr<ID3D12Device5> dxrDevice;
    D3D12_CHECK(m_device.As(&dxrDevice));
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

	ComPtr<ID3D12GraphicsCommandList5> cmdList;
	D3D12_CHECK(a_cmdList.As(&cmdList));
    cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    //#Todo: once queuing for multiple builds ensure that multiple barriers are batched.
	const D3D12_RESOURCE_BARRIER asbarrier =
	{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
		.UAV =
		{
			.pResource = a_as.Buffer.Get()
		}
	};
    cmdList->ResourceBarrier(1, &asbarrier);
}

void CreateBlas(ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as, const ComPtr<ID3D12Resource>& a_vsBuffer, const u32 a_vsCount, ComPtr<ID3D12Resource> a_idBuffer = nullptr, const u32 a_idCount = 0)
{
    const D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE vertexBuffer =
    {
        .StartAddress = a_vsBuffer->GetGPUVirtualAddress(),
        .StrideInBytes = sizeof(fp32) * 3
    };

    //#Todo: Could possibly be retrieved through shader reflection?
    const D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC triangleDesc =
    {
        .Transform3x4 = 0,
        .IndexFormat = a_idCount > 0 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_UNKNOWN,
        .VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
        .IndexCount = a_idCount > 0 ? a_idCount : 0,
        .VertexCount = a_vsCount / 3, //#Note: Divide by 3 as the buffer is made up of raw floats.
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

    CreateAccelerationStructure(a_cmdList, a_as, blasInputs);
}

void CreateTlas(ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_as)
{
	const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs =
	{
        .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
        .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD,
        .NumDescs = m_numBlas,
        .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
        .InstanceDescs = m_blasInstanceBuffer->GetGPUVirtualAddress()
	};

    CreateAccelerationStructure(a_cmdList, a_as, tlasInputs);
}

void CreateGlobalResources()
{
	//#Note: Hack, current commandlist management is limited by the frameloop and does not use pooling, so the first *frame* is spend on resource initialization.
	FrameResources& frameResources = m_frameResources[m_swapchainIndex];

	D3D12_CHECK(frameResources.CommandAllocator->Reset());
	D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));

    CreateReadBackBuffer(m_quadVertexBuffer, m_quadVertices, sizeof(m_quadVertices));
	D3D12_NAME_OBJECT(m_quadVertexBuffer, std::format(L"Quad vertex buffer"));
    CreateReadBackBuffer(m_cubeVertexBuffer, m_cubeVertices, sizeof(m_cubeVertices));
	D3D12_NAME_OBJECT(m_cubeVertexBuffer, std::format(L"Cube vertex buffer"));
    CreateReadBackBuffer(m_cubeIndexBuffer, m_cubeIndicies, sizeof(m_cubeIndicies));
	D3D12_NAME_OBJECT(m_cubeIndexBuffer, std::format(L"Cube index buffer"));

	CreateBlas(m_commandList, m_cubeBlas, m_cubeVertexBuffer, static_cast<u32>(std::size(m_cubeVertices)), m_cubeIndexBuffer, static_cast<u32>(std::size(m_cubeIndicies)));
	D3D12_NAME_OBJECT(m_cubeBlas.Scratch, std::format(L"Cube Blas Scratch"));
	D3D12_NAME_OBJECT(m_cubeBlas.Buffer, std::format(L"Cube Blas"));

	CreateBlas(m_commandList, m_quadBlas, m_quadVertexBuffer, static_cast<u32>(std::size(m_quadVertices)));
	D3D12_NAME_OBJECT(m_cubeBlas.Scratch, std::format(L"Quad Blas Scratch"));
	D3D12_NAME_OBJECT(m_cubeBlas.Buffer, std::format(L"Quad Blas"));

    CreateSceneInstances(); //Depends on the blas being ready to be read.

	D3D12_CHECK(m_commandList->Close());
	ID3D12CommandList* const lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, lists);
	D3D12_CHECK(m_commandQueue->Signal(m_commandQueueFence.Get(), ++m_commandQueueFenceValue));
	WaitForCommandQueueFence(m_commandQueueFenceValue);
}

void CreateRayTraceDemoRootSig(ComPtr<ID3D12RootSignature>& a_rootSig)
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
	D3D12_CHECK(m_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&a_rootSig)));
}

void CreateRayTracingPipelineStateObject(RaytracePipelineStateObject& a_rtpso)
{
    //1. Compile the sahder and create a subobject for it.
	const ShaderCompilationOptions options =
	{
		.OptimizeLevel = EOptimizeLevel::O2,
		.ShaderModel = EShaderModel::SM6_3,
		.ShouldKeepDebugInfo = true
	};

    //#Todo: Move the inline compilation to a different place once abstraction begins.
    const ShaderCompilationOutput compileRes = m_dxShaderCompiler->CompileShader(Path(ENGINE_SHADER_DIRECTORY) / "raytracer.rt.hlsl", options);
    if (compileRes.SizeInBytes <= 0)
    {
        return;
    }

    const D3D12_DXIL_LIBRARY_DESC libDesc =
    {
        .DXILLibrary = 
        {
            compileRes.Data,
            compileRes.SizeInBytes
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
    D3D12_CHECK(dxrDevice->CreateStateObject(&rtpsoDesc, IID_PPV_ARGS(&a_rtpso.Pso)));

    //7. Store the shader Ids so they can be identified when the rays have to be dispatched.
	const D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = a_rtpso.NumShaderIds * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, //#Note: shader ids are 32-byte, the other 32 byte required for alignment can be used for root params.
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

	D3D12_CHECK(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&a_rtpso.ShaderIds)));
    if (!a_rtpso.ShaderIds)
    {
        DXRAY_ERROR("Failed to create rtpso shader ids buffer!");
        return;
    }

	ComPtr<ID3D12StateObjectProperties> psoProps = nullptr;
	D3D12_CHECK(a_rtpso.Pso.As(&psoProps));

    void* data = nullptr;
    a_rtpso.ShaderIds->Map(0, nullptr, &data);

    //#Note: Size doesn't matter here as the shaderids are hardcoded anyways :/
    const std::vector<void*> shaderIds =
    {
	    psoProps->GetShaderIdentifier(L"RayGeneration"),
	    psoProps->GetShaderIdentifier(L"Miss"),
	    psoProps->GetShaderIdentifier(L"HitGroup")
    };

    for (u32 i = 0; i < a_rtpso.NumShaderIds; i++)
    {
		memcpy(data, shaderIds[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		data = static_cast<u8*>(data) + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    }
	
    a_rtpso.ShaderIds->Unmap(0, nullptr);
}


// --- Engine loop ---

void Tick(const fp32 a_dt)
{
	using namespace DirectX;
	D3D12_RAYTRACING_INSTANCE_DESC* const instances = static_cast<D3D12_RAYTRACING_INSTANCE_DESC* const>(m_blasInstanceBufferAddr);

    //Cube.
	XMMATRIX cube = XMMatrixRotationRollPitchYaw(a_dt / 2, a_dt / 3, a_dt / 5);
	cube *= XMMatrixTranslation(-1.5, 2, 2);
    XMFLOAT3X4& ptr = reinterpret_cast<XMFLOAT3X4&>(instances[0].Transform);
	XMStoreFloat3x4(&ptr, cube);

    //Mirror.
    XMMATRIX mirror = XMMatrixRotationX(-1.8f);
	mirror *= XMMatrixRotationY(XMScalarSinEst(a_dt) / 8 + 1);
	mirror *= XMMatrixTranslation(2, 2, 2);
	ptr = reinterpret_cast<XMFLOAT3X4&>(instances[1].Transform);
	XMStoreFloat3x4(&ptr, mirror);

    //Floor.
    XMMATRIX floor = XMMatrixScaling(5, 5, 5);
	floor *= XMMatrixTranslation(0, 0, 2);
	ptr = reinterpret_cast<XMFLOAT3X4&>(instances[2].Transform);
	XMStoreFloat3x4(&ptr, floor);
}

void Render()
{
    // -- Retrieve the data needed to render --
    FrameResources& frameResources = m_frameResources[m_swapchainIndex];
    WaitForCommandQueueFence(frameResources.FenceValue);

	// -- Record the data --
	D3D12_CHECK(frameResources.CommandAllocator->Reset());
    D3D12_CHECK(m_commandList->Reset(frameResources.CommandAllocator.Get(), nullptr));

    // - Rebuild Top level acceleration structures -
    CreateTlas(m_commandList, frameResources.WorldTlas);
	D3D12_NAME_OBJECT(frameResources.WorldTlas.Scratch, std::format(L"WorldBlasScratch"));
	D3D12_NAME_OBJECT(frameResources.WorldTlas.Buffer, std::format(L"WorldBlas"));

    // - Bind all the resources -
    //ComPtr<ID3D12GraphicsCommandList5> dxrCmdList;
    //D3D12_CHECK(m_commandList.As(&dxrCmdList));
    //
    //dxrCmdList->SetPipelineState1(m_rtpso.Pso.Get());
    //dxrCmdList->SetComputeRootSignature(m_rootSig.Get());
    //dxrCmdList->SetDescriptorHeaps(1, &uavHeap);
    //
    //D3D12_GPU_DESCRIPTOR_HANDLE uavTable = uavHeap->GetGPUDescriptorHandleForHeapStart();
    //dxrCmdList->SetComputeRootDescriptorTable(0, uavTable);
    //dxrCmdList->SetComputeRootShaderResourceView(1, m_topLevelAccelerationStructure->GetGPUVirtualAddress());

    // - Dispatch the rays! -



    // - Present - #Todo: move the swapchain render target copy into a separate method - no need to clutter up the render function more.
	ComPtr<ID3D12Resource>& rt = m_swapchainRenderTargets[m_swapchainIndex];
	CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainRenderTargets[m_swapchainIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &bar);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_swapchainIndex, m_rtvDescriptorSize);
	const fp32 clearColor[] = { 200 / 255.0f, 96.0f / 255.f, 24.0f / 255.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    CD3DX12_RESOURCE_BARRIER bar2 = CD3DX12_RESOURCE_BARRIER::Transition(rt.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &bar2);

    D3D12_CHECK(m_commandList->Close());

    // -- Execute the data --
    ID3D12CommandList* const lists[] = { m_commandList.Get() };
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
    fp32 prevFrameSample = 0.0f;
    u32 fps = 0;

	while (m_window->PollEvents())
	{
        const fp32 dt = m_time.GetElapsedSeconds() - prevFrameSample;
        prevFrameSample = m_time.GetElapsedSeconds();
        
        Tick(dt);
        Render();

        elapsedInterval += dt;
		++fps;
		if (elapsedInterval > 1.0f)
		{
			m_window->SetWindowTitle(std::format("{} fps: {} - mspf: {}",
				PROJECT_NAME,
				fps,
				1000.0f / fps
			));
        
			elapsedInterval = 0.0f;
            fps = 0;
        }
	}
}


// --- Entry point ---

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

    CreateDevice(m_device);
    CreateCommandQueue(m_commandQueue, D3D12_COMMAND_LIST_TYPE_DIRECT);
    CreateSwapchain(m_swapchain, windowInfo.Rect.Width, windowInfo.Rect.Height);
    CreateFrameResources();
    CreateGlobalResources();
    CreateRayTraceDemoRootSig(m_rootSig);
    CreateRayTracingPipelineStateObject(m_rtpso);

    FrameLoop();

	WaitForCommandQueueFence(m_commandQueueFence->GetCompletedValue()); //Flush the gpu.
	CloseHandle(m_commandQueueFenceEvent);

	return 0;
}