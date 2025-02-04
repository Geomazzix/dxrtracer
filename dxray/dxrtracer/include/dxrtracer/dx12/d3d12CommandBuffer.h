#pragma once

namespace dxray
{
    /// <summary>
    /// A command buffer represents a series of recorded commands for the GPU, accompanied with it's memory and resource pointers.
    /// The base class implements the copy supported copy commands, which the compute and graphics command-list implementations inherit from.
    /// </summary>
    class D3D12CommandBuffer
    {
    public:
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
    /// The d3d12 command buffer pool owns the lifetime of the d3d12 command buffer pools.
    /// </summary>
    class D3D12CommandBufferPool
    {

    };
}