#include "dxrtracer/dx12/d3d12CommandQueue.h"
#include <core/string.h>

namespace dxray
{
    //--- D3D12CommandAlloctorPool ---

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

    D3D12CommandAllocatorPool::D3D12CommandAllocatorPool() :
        m_type(D3D12_COMMAND_LIST_TYPE_NONE),
        m_device(nullptr)
    { }

    D3D12CommandAllocatorPool::D3D12CommandAllocatorPool(const ComPtr<ID3D12Device>& a_pDevice, const D3D12_COMMAND_LIST_TYPE a_type, u32 a_initialAllocatorCount /*= 4*/) :
        m_device(a_pDevice),
        m_type(a_type)
    {
        m_allocatorPool.reserve(a_initialAllocatorCount);
    }

    D3D12CommandAllocatorPool::~D3D12CommandAllocatorPool()
    {
        while (!m_availableAllocators.empty())
        {
            m_availableAllocators.pop();
        }

        m_allocatorPool.clear();
    }

    ComPtr<ID3D12CommandAllocator> D3D12CommandAllocatorPool::RequestAllocator(const u64 a_completedFenceValue)
    {
        ComPtr<ID3D12CommandAllocator> cmdAllocator = nullptr;

        //First check if there is an allocator that's ready to be reused.
        if (!m_availableAllocators.empty())
        {
            std::pair<u64, ComPtr<ID3D12CommandAllocator>>& allocatorPair = m_availableAllocators.front();
            if (allocatorPair.first <= a_completedFenceValue)
            {
                cmdAllocator = allocatorPair.second;
                D3D12_CHECK(cmdAllocator->Reset());
                m_availableAllocators.pop();
            }
        }

        //If non was available create a new one.
        if (cmdAllocator == nullptr)
        {
            D3D12_CHECK(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&cmdAllocator)));
            D3D12_NAME_OBJECT(cmdAllocator, std::format(L"D3D12{}CommandAllocator_{}", CommandListTypeToUnicode(m_type), m_allocatorPool.size()));
            m_allocatorPool.push_back(cmdAllocator);
        }

        return cmdAllocator;
    }

    void D3D12CommandAllocatorPool::DiscardAllocator(const u64 a_fenceValue, const ComPtr<ID3D12CommandAllocator>& a_pAllocator)
    {
        m_availableAllocators.push(std::make_pair(a_fenceValue, a_pAllocator));
    }


    //--- D3D12CommandQueue ---

    D3D12CommandQueue::D3D12CommandQueue(const ComPtr<ID3D12Device>& a_pDevice, const ECommandQueueType a_type) :
        m_device(a_pDevice),
        m_type(a_type),
        m_fence(nullptr),
        m_commandQueue(nullptr),
        m_nextFenceValue(0),
        m_fenceEventHandle(nullptr)
    {
        const D3D12_COMMAND_LIST_TYPE apiType = a_type == ECommandQueueType::Present
            ? static_cast<D3D12_COMMAND_LIST_TYPE>(ECommandQueueType::Graphics)
            : static_cast<D3D12_COMMAND_LIST_TYPE>(a_type);

        m_allocatorPool = D3D12CommandAllocatorPool(a_pDevice, apiType, 4);

        const D3D12_COMMAND_QUEUE_DESC queueInfo =
        {
            .Type = apiType,
            .Priority = 0,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0
        };

        D3D12_CHECK(a_pDevice->CreateCommandQueue(&queueInfo, IID_PPV_ARGS(&m_commandQueue)));        
        D3D12_NAME_OBJECT(m_commandQueue, std::format(L"D3D12{}CommandQueue", CommandListTypeToUnicode(apiType)));

        D3D12_CHECK(a_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        D3D12_NAME_OBJECT(m_fence, std::format(L"D3D12{}CommandQueue_Fence", CommandListTypeToUnicode(apiType)));
        m_fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    }

    D3D12CommandQueue::~D3D12CommandQueue()
    {
        WaitIdle();
        CloseHandle(m_fenceEventHandle);
        m_nextFenceValue = 0;
    }

    void D3D12CommandQueue::WaitForFence(const u64 a_fenceValue)
    {
        if (IsFenceComplete(a_fenceValue))
        {
            return;
        }

        D3D12_CHECK(m_fence->SetEventOnCompletion(a_fenceValue, m_fenceEventHandle));
        WaitForSingleObject(m_fenceEventHandle, INFINITE);
    }

    u64 D3D12CommandQueue::IncrementFence()
    {
        D3D12_CHECK(m_commandQueue->Signal(m_fence.Get(), ++m_nextFenceValue));
        return m_nextFenceValue;
    }

    bool D3D12CommandQueue::IsFenceComplete(const u64 a_fenceValue)
    {
        return m_nextFenceValue <= a_fenceValue;
    }
}