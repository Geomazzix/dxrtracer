#include "dxrtracer/dx12/d3d12Command.h"

namespace dxray
{
    //--- D3D12CommandAlloctorPool ---

    inline WString CommandListTypeToUnicode(const D3D12_COMMAND_LIST_TYPE a_type)
    {
        switch (a_type)
        {
        D3D12_COMMAND_LIST_TYPE_DIRECT: return L"Direct";
        D3D12_COMMAND_LIST_TYPE_COMPUTE: return L"Compute";
        D3D12_COMMAND_LIST_TYPE_BUNDLE: return L"Bundle";
        D3D12_COMMAND_LIST_TYPE_COPY: return L"Copy";
        default:
        }

        return L"Unidentified";
    }

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

    const ComPtr<ID3D12CommandAllocator>& D3D12CommandAllocatorPool::RequestAllocator(const u64 a_completedFenceValue)
    {
        ComPtr<ID3D12CommandAllocator> pCmdAllocator = nullptr;

        //First check if there is an allocator that's ready to be reused.
        if (!m_availableAllocators.empty())
        {
            std::pair<u64, ComPtr<ID3D12CommandAllocator>>& allocatorPair = m_availableAllocators.front();
            if (allocatorPair.first <= a_completedFenceValue)
            {
                pCmdAllocator = allocatorPair.second;
                D3D12_CHECK(pCmdAllocator->Reset());
                m_availableAllocators.pop();
            }
        }

        //If non was available create a new one.
        if (pCmdAllocator == nullptr)
        {
            D3D12_CHECK(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&pCmdAllocator)));
            pCmdAllocator->SetName(std::format(L"D3D12{}CommandAllocator_{}", CommandListTypeToUnicode(m_type), m_allocatorPool.size()).c_str());
            m_allocatorPool.push_back(pCmdAllocator);
        }

        return pCmdAllocator;
    }

    void D3D12CommandAllocatorPool::DiscardAllocator(const u64 a_fenceValue, const ComPtr<ID3D12CommandAllocator>& a_pAllocator)
    {
        m_availableAllocators.push(std::make_pair(a_fenceValue, a_pAllocator));
    }


    //--- D3D12CommandQueue ---


    D3D12CommandQueue::D3D12CommandQueue(const ComPtr<ID3D12Device>& a_pDevice, const ECommandQueueType a_type) :
        m_device(a_pDevice),
        m_type(a_type),
        m_allocatorPool(a_pDevice, a_type == ECommandQueueType::Present 
            ? static_cast<D3D12_COMMAND_LIST_TYPE>(ECommandQueueType::Graphics) 
            : static_cast<D3D12_COMMAND_LIST_TYPE>(a_type), 4)
    {

    }

    D3D12CommandQueue::~D3D12CommandQueue()
    {

    }
}