#pragma once

namespace dxray
{
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
    /// </summary>
    class D3D12CommandBuffer
    {
    public:
        D3D12CommandBuffer();
        ~D3D12CommandBuffer();

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
        ComPtr<ID3D12CommandList> m_commandBuffer;
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
		D3D12CommandBufferPool();
		~D3D12CommandBufferPool();

		ComPtr<ID3D12CommandAllocator> RequestCommandList(const u64 a_completedFenceValue);
		void RecycleCommandList(const u64 a_fenceValue, const ComPtr<ID3D12CommandAllocator>& a_pAllocator);

	private:
		std::vector<std::shared_ptr<D3D12CommandBuffer>> m_allocatorPool[static_cast<i32>(ECommandBufferType::Count)];
		std::queue<std::pair<u64, std::shared_ptr<D3D12CommandBuffer>>> m_availableAllocators;
	};
}