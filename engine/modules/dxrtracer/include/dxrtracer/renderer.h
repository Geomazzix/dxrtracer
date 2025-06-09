#pragma once
#include <core/valueTypes.h>
#include <core/containers/array.h>
#include "dxrtracer/accelerationStructure.h"

namespace dxray
{
	class WinApiWindow;
	class Scene;
	class RenderPass;
	struct Model;

	/**
	* @brief Render data of a singular scene render-able - this data can and should be instanced.
	*/
	struct SceneObjectRenderData
	{
		ComPtr<ID3D12Resource> VertexBuffer = nullptr;
		ComPtr<ID3D12Resource> IndexBuffer = nullptr;
		AccelerationStructure Blas;
	};


	/**
	 * @brief Global frame-to-frame resources, these exist x amount of swap-chain back-buffers.
	 */
	struct FrameResources
	{
		ComPtr<ID3D12Resource> WorldTlasInstancesData = nullptr;
		AccelerationStructure WorldTlas;

		ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
		ComPtr<ID3D12Resource> RaytraceRenderTarget = nullptr;
		u64 FenceValue = 0;
	};


	/**
	 * @brief Wrapper object for a command queue - internally tracks submission and synchronization based on provided primitives.
	 */
	struct D3d12CommandQueue
	{
		D3d12CommandQueue();
		~D3d12CommandQueue();

		ComPtr<ID3D12CommandQueue> Handle;
		ComPtr<ID3D12Fence> Fence;
		HANDLE FenceEvent;
		u64 FenceValue;

		u64 Signal();
		void WaitForFence(const u64 a_fenceValue);
		void WaitIdle();
	};


	/**
	 * @brief Info structure to describe the swap chain properties during (re-)creation.
	 */
	struct SwapchainCreateInfo
	{
		u32 SurfaceWidth;
		u32 SurfaceHeight;
		std::shared_ptr<WinApiWindow> Window;
	};


	/**
	 * @brief Info structure to describe the renderer initialization.
	 */
	struct RendererCreateInfo
	{
		SwapchainCreateInfo SwapchainInfo;
	};


	/**
	 * @brief The renderer is responsible for the GPU frame-to-frame generation and execution, while maintaining the render-pass dependencies as well.
	 */
	class Renderer
	{
	public:
		Renderer(const RendererCreateInfo& a_createInfo);
		~Renderer();

		void BeginResourceLoading();
		void LoadModel(std::shared_ptr<Scene>& a_pScene, Model& a_model);
		void EndResourceLoading();
		void Render(std::shared_ptr<Scene>& a_pScene);

	private:
		void Present(ComPtr<ID3D12Resource>& a_renderTargetOutput);
		
		void CreateDevice();
		void CreateCommandQueue(D3d12CommandQueue& a_commandQueue, const D3D12_COMMAND_LIST_TYPE a_type);
		void CreateSwapchain(const SwapchainCreateInfo& a_swapchainCreateInfo);
		void CreateFrameResources();

		void CreateModelResources(ComPtr<ID3D12GraphicsCommandList>& a_cmdList, const Model& a_model);

		bool m_bUseWarp;
		ComPtr<IDXGIFactory4> m_factory;
		ComPtr<ID3D12Device> m_device;

		const static u32 SwapchainBackbufferCount = 3;
		FixedArray<ComPtr<ID3D12Resource>, SwapchainBackbufferCount> m_swapchainRenderTargets;
		FixedArray<FrameResources, SwapchainBackbufferCount> m_frameResources;
		u32 m_swapchainIndex = 0;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
		u32 m_rtvDescriptorSize = 0;
		ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

		ComPtr<ID3D12DescriptorHeap> m_uavHeap = nullptr;
		u32 m_uavDescriptorSize = 0;

		D3d12CommandQueue m_graphicsQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;

		std::unique_ptr<RenderPass> m_renderPass;
		Array<SceneObjectRenderData> m_sceneObjectRenderDataBuffer;
	};
}