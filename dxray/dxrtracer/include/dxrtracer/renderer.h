#pragma once

namespace dxray
{
	class D3D12Device;

	struct RendererCreateInfo
	{
		vath::Rectu32 WindowRect;
		void* WindowHandle;
		bool bUseWarp = false;
	};

	class Renderer
	{
	public:
		Renderer(const RendererCreateInfo& a_info);
		~Renderer();

		void Render();

	private:
		std::unique_ptr<D3D12Device> m_device;
	};
}