#include "dxrtracer/renderer.h"
#include "dxrtracer/dx12/d3d12Device.h"
#include "dxrtracer/dx12/d3d12Command.h"

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

	}
}