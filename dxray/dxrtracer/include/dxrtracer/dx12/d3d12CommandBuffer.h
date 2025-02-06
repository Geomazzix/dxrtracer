#pragma once

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

    /// <summary>
    /// Used to identify the command list in various scenarios.
    /// #Note: can be casted between ECommandQueueType as long as the names alias one another (e.g. ECommandBufferType::Graphics <-> ECommandQueueType::Graphics).
    /// </summary>
    enum class ECommandBufferType : i8
    {
		Graphics = D3D12_COMMAND_LIST_TYPE_DIRECT,
		Bundle = D3D12_COMMAND_LIST_TYPE_BUNDLE,
		Compute = D3D12_COMMAND_LIST_TYPE_COMPUTE,
		Copy = D3D12_COMMAND_LIST_TYPE_COPY,
        Invalid = D3D12_COMMAND_LIST_TYPE_NONE,
        Count = Copy
    };


    /// <summary>
    /// A command buffer represents a series of recorded commands for the GPU, accompanied with it's memory and resource pointers.
    /// The base class implements the copy supported copy commands, which the compute and graphics command-list implementations inherit from.
    /// #Note: These are stored in a command queue, meaning their type is lifetime defined.
    /// </summary>
    class D3D12CommandBuffer
    {
    public:
        D3D12CommandBuffer() = default;
        D3D12CommandBuffer(const ECommandBufferType a_type, ComPtr<ID3D12GraphicsCommandList> a_pCommandBuffer, ComPtr<ID3D12CommandAllocator> a_pCommandAllocator) :
            m_commandBuffer(a_pCommandBuffer),
            m_commandAllocator(a_pCommandAllocator),
            m_type(a_type)
        {

        }

        virtual ~D3D12CommandBuffer() = default;

        void Reset(const ComPtr<ID3D12CommandAllocator>& a_pAllocator)
        {
            m_commandAllocator = a_pAllocator;
            m_commandBuffer->Reset(m_commandAllocator.Get(), nullptr);
        }

        void Close()
        {
            D3D12_CHECK(m_commandBuffer->Close());
        }

        ECommandBufferType GetType() const
        {
            return m_type;
        }

        ComPtr<ID3D12GraphicsCommandList> GetCommandBuffer()
        {
            DXRAY_ASSERT(m_commandBuffer != nullptr);
            return m_commandBuffer;
        }

        ComPtr<ID3D12CommandAllocator> GetCommandAllocator()
        {
            DXRAY_ASSERT(m_commandAllocator != nullptr);
            return m_commandAllocator;
        }

        /*
        Copy commands:
        - Close
        - CopyBufferRegion
        - CopyResource
        - CopyTextureRegion
        - CopyTiles
        - Reset
        - ResourceBarrier
        */


    private:
        ComPtr<ID3D12GraphicsCommandList> m_commandBuffer;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ECommandBufferType m_type;
    };


    /// <summary>
    /// Able to execute compute and copy commands.
    /// </summary>
    class D3D12ComputeCommandBuffer final : public D3D12CommandBuffer
    {
        /*
        Compute commands (and copy commands mentioned above):
        - ClearState
        - ClearUnorderedAccessViewFloat
        - ClearUnorderedAccessViewUint
        - DiscardResource
        - Dispatch
        - ExecuteIndirect
        - SetComputeRoot32BitConstant
        - SetComputeRoot32BitConstants
        - SetComputeRootConstantBufferView
        - SetComputeRootDescriptorTable
        - SetComputeRootShaderResourceView
        - SetComputeRootSignature
        - SetComputeRootUnorderedAccessView
        - SetDescriptorHeaps
        - SetPipelineState
        - SetPredication
        - EndQuery
        */

    };


    /// <summary>
    /// Able to execute graphics and copy commands.
    /// </summary>
    class D3D12GraphicsCommandBuffer final : public D3D12CommandBuffer
    {
        //https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12graphicscommandlist
    };


	/// <summary>
	/// The d3d12 command buffer pool owns x amount of command list type pools that can be reused through a queue-like system.
	/// #Note: This approach is an attempt to spread out the workload of the different type of command lists over different queues, it is by no means well tested on efficiency.
	/// </summary>
	class D3D12CommandBufferPool final
	{
	public:
        D3D12CommandBufferPool() :
            m_device(nullptr),
            m_type(ECommandBufferType::Invalid)
        { }

        D3D12CommandBufferPool(const ComPtr<ID3D12Device>& a_pDevice, const ECommandBufferType& a_type, const u32 a_initialCommandBufferCount) :
            m_device(a_pDevice),
            m_type(a_type)
        {
            m_commandBufferPool.reserve(a_initialCommandBufferCount);
        }
        
        ~D3D12CommandBufferPool()
        {
            while (!m_availableCommandBuffers.empty())
            {
                m_availableCommandBuffers.pop();
            }

            m_commandBufferPool.clear();
        }

		std::shared_ptr<D3D12CommandBuffer> RequestCommandBuffer(const u64 a_completedFenceValue, const ComPtr<ID3D12CommandAllocator>& a_pAllocator)
        {
            std::shared_ptr<D3D12CommandBuffer> cmdBuffer = nullptr;
            if (!m_availableCommandBuffers.empty())
            {
                std::pair<u64, std::shared_ptr<D3D12CommandBuffer>>& commandBufferPair = m_availableCommandBuffers.front();
                if (commandBufferPair.first <= a_completedFenceValue)
                {
                    cmdBuffer = commandBufferPair.second;
                    cmdBuffer->Reset(a_pAllocator);
                    m_availableCommandBuffers.pop();
                }
            }

            if(cmdBuffer == nullptr)
            {
                ComPtr<ID3D12GraphicsCommandList> d3d12CmdBuffer = nullptr;
                const D3D12_COMMAND_LIST_TYPE type = static_cast<D3D12_COMMAND_LIST_TYPE>(m_type);
                D3D12_CHECK(m_device->CreateCommandList(0, type, a_pAllocator.Get(), nullptr, IID_PPV_ARGS(&d3d12CmdBuffer)));
                D3D12_NAME_OBJECT(d3d12CmdBuffer, std::format(L"D3D12{}CommandBuffer{}", CommandListTypeToUnicode(type), m_commandBufferPool.size()));
                cmdBuffer = std::make_shared<D3D12CommandBuffer>(m_type, d3d12CmdBuffer, a_pAllocator);
                m_commandBufferPool.push_back(cmdBuffer);
            }

            return cmdBuffer;
        }

        void DiscardCommandList(const u64 a_fenceValue, const std::shared_ptr<D3D12CommandBuffer>& a_pCommandBuffer)
        {
            m_availableCommandBuffers.push(std::make_pair(a_fenceValue, a_pCommandBuffer));
        }

	private:
		std::vector<std::shared_ptr<D3D12CommandBuffer>> m_commandBufferPool;
		std::queue<std::pair<u64, std::shared_ptr<D3D12CommandBuffer>>> m_availableCommandBuffers;
	
        ComPtr<ID3D12Device> m_device;
        ECommandBufferType m_type;
    };
}