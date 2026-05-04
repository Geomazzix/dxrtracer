#pragma once
#include "dxrtracer/descriptorHeap.h"

namespace dxray
{
	/**
	 * @brief PSO wrapper object to ensure correct shader binding during ray dispatch in the renderpass.
	 */
	struct RaytracePipelineStateObject
	{
		ComPtr<ID3D12StateObject> Pso;
		ComPtr<ID3D12Resource> ShaderTable;
	};

	/**
	 * @brief Render pass execution info for the dispatch rays.
	 */
	struct RenderPassExecuteInfo
	{
		const DescriptorHeap& BindlessHeap;
		D3D12_GPU_VIRTUAL_ADDRESS SceneCbvAddr;
		D3D12_GPU_VIRTUAL_ADDRESS TlasBufferAddr;
		u32 RenderTargetUavSlot;
		u32 SurfaceWidth;
		u32 SurfaceHeight;
	};

	/**
	 * @brief The render-pass is currently responsible for the dispatching of the rays for the ray-traced output.
	 */
	class RenderPass
	{
	public:
		RenderPass(ComPtr<ID3D12Device> a_device);
		~RenderPass() = default;

		void Execute(ComPtr<ID3D12GraphicsCommandList>& a_commandList, const RenderPassExecuteInfo& a_execInfo);

	private:
		void CreateRayTraceDemoRootSig();
		void CreateRayTracingPipelineStateObject();
		void CreateShaderTable();

		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12RootSignature> m_rootSig;
		RaytracePipelineStateObject m_rtpso;
	};
}