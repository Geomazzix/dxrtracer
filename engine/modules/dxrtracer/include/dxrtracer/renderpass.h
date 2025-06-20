#pragma once

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
		ComPtr<ID3D12DescriptorHeap>& UavHeap;
		ComPtr<ID3D12Resource>& TlasBuffer;
		ComPtr<ID3D12Resource>& SceneCbv;

		u32 SwapchainIndex;
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

	public:
		void CreateRayTraceDemoRootSig();
		void CreateRayTracingPipelineStateObject();
		void CreateShaderTable();

		RaytracePipelineStateObject m_rtpso;
		ComPtr<ID3D12RootSignature> m_rootSig;
		ComPtr<ID3D12Device> m_device;
	};
}