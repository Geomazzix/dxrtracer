#pragma once
#include <core/valueTypes.h>
#include <core/containers/array.h>
#include <core/memory/memory.h>
#include "dxrtracer/shaderConstructs.h"
#include "dxrtracer/accelerationStructure.h"

namespace dxray
{
	class WinApiWindow;
	class RenderPass;
	struct Camera;
	struct Model;

	/**
	 * @brief Used to indicate which sample size the shader should use.
	 * @Note Kept 4 bytes big, as hlsl doesn't support 8 bit types.
	 */
	enum class ESuperSampleSize : u32
	{
		x1 = 1,
		x2 = 2,
		x4 = 4,
		x8 = 8
	};


	/**
	 * @brief Constantbuffers require alignment to 255 bytes.
	 * @param a_size the size of the constant buffer to be aligned.
	 * @return A 255 aligned size.
	 */
	inline usize CalculateConstantBufferSize(const usize a_size)
	{
		return Align<usize>(a_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	}


	/**
	* @brief Render data as represented by d3d12. Not instanced.
	*/
	struct D3D12Mesh
	{
		ComPtr<ID3D12Resource> VertexPositionAttribBuffer = nullptr;
		ComPtr<ID3D12Resource> VertexNormalAttribBuffer = nullptr;
		ComPtr<ID3D12Resource> VertexUvAttribBuffer = nullptr;
		usize VertexCount = 0;

		ComPtr<ID3D12Resource> IndexBuffer = nullptr;
		usize IndexCount = 0;

		BottomLevelAccelerationStructure Blas;
	};


	/**
	 * @brief Global frame-to-frame resources, these exist x amount of swap-chain back-buffers.
	 */
	struct FrameResources
	{
		SceneConstantBuffer SceneConstantBufferData;
		TopLevelAccelerationStructure WorldTlas;

		ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
		ComPtr<ID3D12Resource> RaytraceRenderTarget = nullptr;
		u64 FenceValue = 0;
	};


	/**
	 * @brief Wrapper object for a command queue - internally tracks submission and synchronization based on provided primitives.
	 */
	struct CommandQueue
	{
		CommandQueue();
		~CommandQueue();

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
		std::shared_ptr<Camera> MainCam;
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
		void LoadModel(const vath::Vector3f& a_location, const vath::Vector3f& a_eulerRotation, const vath::Vector3f& a_scale, const Model& a_model);
		void EndResourceLoading();
		void Render(const fp32 a_dt); // #Todo: Remove dt here -> left in for debugging though should move to tick only when camera update is abstracted.

	private:
		void Present(ComPtr<ID3D12Resource>& a_renderTargetOutput);
		
		void CreateDevice();
		void CreateCommandQueue(CommandQueue& a_commandQueue, const D3D12_COMMAND_LIST_TYPE a_type);
		void CreateSwapchain(const SwapchainCreateInfo& a_swapchainCreateInfo);
		void CreateFrameResources();

		std::shared_ptr<Camera> m_mainCamera;
		Array<D3D12Mesh> m_meshes;
		Array<D3D12_RAYTRACING_INSTANCE_DESC> m_sceneObjectInstances;

		// #Todo: Looks this like this resource, regardless of whether the GPU is idle or not, is not being correctly destroyed...? - keep an eye on this.
		ComPtr<ID3D12Resource> m_cbvSceneHeap;
		usize m_alignedSceneConstantBufferElementSize;
		void* m_cbvSceneHeapAddr;
		ComPtr<ID3D12DescriptorHeap> m_geometryDescriptors;

		const static u32 SwapchainBackbufferCount = 3;
		FixedArray<ComPtr<ID3D12Resource>, SwapchainBackbufferCount> m_swapchainRenderTargets;
		FixedArray<FrameResources, SwapchainBackbufferCount> m_frameResources;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_uavHeap;
		ComPtr<IDXGISwapChain3> m_swapchain;
		u32 m_swapchainIndex;
		u32 m_rtvDescriptorSize;
		u32 m_uavCbvSrvDescriptorSize;
		bool m_useWarp;

		std::unique_ptr<RenderPass> m_renderPass;
		std::unique_ptr<CommandQueue> m_graphicsQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;

		ComPtr<IDXGIFactory4> m_factory;
		ComPtr<ID3D12Device> m_device;
	};
}