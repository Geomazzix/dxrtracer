#pragma once
#include <core/string.h>

namespace dxray
{
	class WinApiWindow;
	class D3D12Device;

	struct ApplicationCreateInfo
	{
		String Title = String(PROJECT_NAME);
		vath::Rectu32 Rect = vath::Rectu32(0, 0, 1600, 900);
	};

	class DxrTracer final
	{
	public:
		DxrTracer(const ApplicationCreateInfo& a_info);
		~DxrTracer() = default;

	private:
		void EngineLoop();

		std::unique_ptr<WinApiWindow> m_window;
		std::unique_ptr<D3D12Device> m_graphicsDevice;
	};
}