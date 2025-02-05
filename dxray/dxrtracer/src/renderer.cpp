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

        //Record commandlists.

        m_device->Present();
        frameFenceValues[currentFrameIndex] = m_device->GetGraphicsQueue()->IncrementFence();
    }
}