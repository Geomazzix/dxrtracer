#pragma once

using Microsoft::WRL::ComPtr;

#define D3D12_CHECK(hr) DXRAY_ASSERT(SUCCEEDED(hr))

namespace dxray
{
	struct Vertex
	{
		vath::Vector3f Position;
		vath::Vector4f Color;
	};

	struct RendererCreateInfo
	{
		bool bUseWarp = false;
		vath::Rectf RenderRect;
		vath::Vector2f RenderDepthLimits = vath::Vector2f(0.001f, 2000.0f);
		void* WindowHandle;
	};

	class Renderer
	{
	public:
		Renderer(const RendererCreateInfo& a_info);
		~Renderer();

		void Render();

	private:
		static const u32 FrameCount = 2;

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		
		ComPtr<IDXGIFactory4> m_factory;
		ComPtr<ID3D12Device> m_device;
		ComPtr<IDXGISwapChain3> m_swapChain;
		std::array<ComPtr<ID3D12Resource>, FrameCount> m_renderTargets;
		
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		u32 m_rtvDescriptorSize;

		u32 m_frameIndex;
		void* m_fenceEvent;
		u64 m_fenceValue;
		bool m_bUseWarp;
		void* m_windowHandle;

		void GetHardwareAdapter(IDXGIFactory2* a_pFactory, IDXGIAdapter1** a_ppAdapter);
		void CreateSwapchain();
		void LoadAssets();
		void PopulateCommandList();
		void WaitForPreviousFrame();
	};
}