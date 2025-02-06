#include "dxrtracer/renderer.h"
#include "dxrtracer/dx12/d3d12Device.h"

//#Todo: Add customizability for vsync, currently it's turned on thereby preventing different monitor refresh rates to dynamically adjust.

namespace dxray
{
	Renderer::Renderer(const RendererCreateInfo& a_info)
	{
        const DeviceCreateInfo deviceInfo =
        {
            .SwapchainInfo =
            {
                .SwapchainRect = a_info.WindowRect,
                .WindowHandle = a_info.WindowHandle
            },
            .bUseWarp = false
        };
        
        m_device = std::make_unique<D3D12Device>(deviceInfo);
	}

	Renderer::~Renderer()
	{

	}

	void Renderer::Render()
	{
        //#Todo: change swapchain backend to be 3 backbuffers, instead of 2.
        static u64 frameFenceValues[2] = { 0, 1 };
        const u32 currentFrameIndex = m_device->GetFrameIndex();

        m_device->GetGraphicsQueue()->WaitForFence(frameFenceValues[currentFrameIndex]);
        std::shared_ptr<D3D12CommandBuffer> cmdBuffer = m_device->RequestCommandBuffer(ECommandBufferType::Graphics);

        //const ComPtr<ID3D12GraphicsCommandList>& s = cmdBuffer->GetCommandBuffer();
        //
        //D3D12_VIEWPORT viewport =
        //{
        //    .TopLeftX = 0,
        //    .TopLeftY = 0,
        //    .Width = 1600,
        //    .Height = 900,
        //    .MinDepth = 0.001f,
        //    .MaxDepth = 1000.0f
        //};
        //s->RSSetViewports(1, &viewport);
        //
        //RECT scissorRect =
        //{
        //    .left = 0,
        //    .top = 0,
        //    .right = 1600,
        //    .bottom = 900
        //};
        //s->RSSetScissorRects(1, &scissorRect);

        frameFenceValues[currentFrameIndex] = m_device->ExecuteCommandLIst(cmdBuffer);
        m_device->Present();
    }
}