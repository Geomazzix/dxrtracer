#pragma once
#include "dxrtracer/dx12/d3d12CommandQueue.h"
#include "dxrtracer/dx12/d3d12CommandBuffer.h"

//#Todo: See if the queue getters can potentially be const refs to prevent potentially expensive std::shared_ptr copy constructors.

namespace dxray
{
	struct SwapchainCreateInfo
	{
		vath::Rectu32 SwapchainRect;
		void* WindowHandle;
	};

	struct DeviceCreateInfo
	{
		SwapchainCreateInfo SwapchainInfo;
		bool bUseWarp;
	};

	/// <summary>
	/// Responsible for:
	/// - Swap chain management
	/// - Command buffer retrieval, submissions and command queue synchronization.
	/// </summary>
	class D3D12Device
	{
	public:
		D3D12Device(const DeviceCreateInfo& a_info);
		~D3D12Device();

		std::shared_ptr<D3D12CommandQueue> GetGraphicsQueue();
		std::shared_ptr<D3D12CommandQueue> GetComputeQueue();
		std::shared_ptr<D3D12CommandQueue> GetCopyQueue();
		std::shared_ptr<D3D12CommandQueue> GetQueue(const ECommandQueueType a_type);

		std::shared_ptr<D3D12CommandBuffer> GetCommandBuffer();
		void SubmitCommandBuffers(std::shared_ptr<D3D12CommandBuffer>* const a_pCommandBuffers, const u32 a_numCommandBuffers);

		void BeginFrame();
		void EndFrame();

		void Present();
		void WaitIdle();

		u32 GetFrameIndex() const;

	private:
        void CreateSwapchain(const SwapchainCreateInfo& a_swapchainInfo);

		static const u32 FrameCount = 2;
        ComPtr<ID3D12Device> m_device;

		std::shared_ptr<D3D12CommandQueue> m_presentQueue;
        std::shared_ptr<D3D12CommandQueue> m_graphicsQueue;
        std::shared_ptr<D3D12CommandQueue> m_computeQueue;
        std::shared_ptr<D3D12CommandQueue> m_copyQueue;

		u32 m_rtvDescriptorSize;
		u32 m_swapchainIndex;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<IDXGIFactory4> m_factory;
		ComPtr<IDXGISwapChain3> m_swapchain;
		std::array<ComPtr<ID3D12Resource>, FrameCount> m_renderTargets;


		bool m_bUseWarp;
	};

	inline std::shared_ptr<D3D12CommandQueue> D3D12Device::GetGraphicsQueue()
	{
		return m_graphicsQueue;
	}

    inline std::shared_ptr<D3D12CommandQueue> D3D12Device::GetComputeQueue()
    {
        return m_computeQueue;
    }

    inline std::shared_ptr<D3D12CommandQueue> D3D12Device::GetCopyQueue()
    {
        return m_copyQueue;
    }

	inline std::shared_ptr<D3D12CommandQueue> D3D12Device::GetQueue(const ECommandQueueType a_type)
	{
		switch (a_type)
		{
		case ECommandQueueType::Graphics:
		{
			return m_graphicsQueue;
		}
        case ECommandQueueType::Present:
		{
            return m_presentQueue;
		}
		case ECommandQueueType::Compute:
		{
			return m_computeQueue;
		}
		case ECommandQueueType::Copy:
		{
			return m_copyQueue;
		}
		case ECommandQueueType::Decode:
		case ECommandQueueType::Process:
		case ECommandQueueType::Encode:
		{
			DXRAY_ASSERT("The device has no implementation for the requested queue type.");
			return nullptr;
		};
		case ECommandQueueType::Invalid:
		default:
            DXRAY_ASSERT("Invalid queue type requested, d3d12 does not support this type.");
            return nullptr;
		}

		return nullptr;
	}

	inline u32 D3D12Device::GetFrameIndex() const
	{
		return m_swapchainIndex;
	}
}