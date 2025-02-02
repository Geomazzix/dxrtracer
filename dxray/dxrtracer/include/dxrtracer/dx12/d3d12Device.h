#pragma once

using Microsoft::WRL::ComPtr;
#define D3D12_CHECK(hr) DXRAY_ASSERT(SUCCEEDED(hr))

namespace dxray
{
	struct SwapchainCreateInfo
	{
		vath::Rectu32 RenderTargetRect;
		void* WindowHandle;
	};

	struct DeviceCreateInfo
	{
		SwapchainCreateInfo SwapchainInfo;
		bool bUseWarp;
	};

	/// <summary>
	/// The D3D12Device is responsible for:
	/// - Swapchain creation/destruction/resizing.
	/// - Resource creation/destruction and mapping/unmapping.
	/// - Queue submission.
	/// </summary>
	class D3D12Device
	{
	public:
		D3D12Device(const DeviceCreateInfo& a_info);
		~D3D12Device();

	private:
		static const u32 FrameCount = 2;

		ComPtr<ID3D12Device> m_device;
		ComPtr<IDXGIFactory4> m_factory;

		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		std::array<ComPtr<ID3D12Resource>, FrameCount> m_renderTargets;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		u32 m_rtvDescriptorSize;
		u32 m_swapchainIndex;

		bool m_bUseWarp;

		void CreateSwapchain(const SwapchainCreateInfo& a_swapchainInfo);
	};
}