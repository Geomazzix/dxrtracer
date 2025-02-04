#pragma once

namespace dxray
{
    /// <summary>
    /// Custom definition for the command queue to identify which queue can be used to initialize the swapchain.
    /// </summary>
    enum class ECommandQueueType : i8
    {
        Graphics = D3D12_COMMAND_LIST_TYPE_DIRECT,
        Compute = D3D12_COMMAND_LIST_TYPE_COMPUTE,
        Copy = D3D12_COMMAND_LIST_TYPE_COPY,
        Decode = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
        Process = D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
        Encode = D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
        Present = D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE + 1, //Custom present queue type for the swap chain and deferred presentation.
        Invalid = -1
    };


    /// <summary>
    /// Command allocator pool, to store and reuse ID3D12CommandAllocators.
    /// Uses the queue to retrieve available allocators, while storing them in the vector to prevent them from destructing.
    /// #Note: Currently not thread safe.
    /// </summary>
    class D3D12CommandAllocatorPool
    {
    public:
        D3D12CommandAllocatorPool(const ComPtr<ID3D12Device>& a_pDevice, const D3D12_COMMAND_LIST_TYPE a_type, u32 a_initialAllocatorCount = 4);
        ~D3D12CommandAllocatorPool();

        const ComPtr<ID3D12CommandAllocator>& RequestAllocator(const u64 a_completedFenceValue);
        void DiscardAllocator(const u64 a_fenceValue, const ComPtr<ID3D12CommandAllocator>& a_pAllocator);

    private:
        const D3D12_COMMAND_LIST_TYPE m_type;
        ComPtr<ID3D12Device> m_device;

        std::vector<ComPtr<ID3D12CommandAllocator>> m_allocatorPool;
        std::queue<std::pair<u64, ComPtr<ID3D12CommandAllocator>>> m_availableAllocators;
    };


    /// <summary>
    /// The command queue manages command buffer submission and ensures synchronization between CPU and GPU between submissions.
    /// #Note: Currently not thread safe.
    /// </summary>
    class D3D12CommandQueue
    {
    public:
        D3D12CommandQueue(const ComPtr<ID3D12Device>& a_pDevice, const ECommandQueueType a_type);
        ~D3D12CommandQueue();

        u64 IncrementFence();
        bool IsFenceComplete(const u64 a_fenceValue);

        void WaitForFence(const u64 a_fenceValue);
        void WaitForIdle();

        u64 GetFenceValue() const;
        u64 ExecuteCommandList(const ComPtr<ID3D12CommandList>& a_pCmdList);

        ComPtr<ID3D12CommandQueue> GetPresentQueue();

    private:
        D3D12CommandAllocatorPool m_allocatorPool;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        const ECommandQueueType m_type;

        ComPtr<ID3D12Fence> m_pFence;
        u64 m_nextFenceValue;
        u64 m_lastCompletedFenceValue;
        HANDLE m_fenceEventHandle;
    };

    inline ComPtr<ID3D12CommandQueue> D3D12CommandQueue::GetPresentQueue()
    {
        DXRAY_ASSERT(m_type == ECommandQueueType::Present);
        return m_type == ECommandQueueType::Present ? m_commandQueue : nullptr;
    }

    inline void D3D12CommandQueue::WaitForIdle()
    {
        WaitForFence(IncrementFence());
    }

    inline u64 D3D12CommandQueue::GetFenceValue() const
    {
        return m_nextFenceValue;
    }
}