#pragma once
#include <core/string.h>

namespace dxray
{
	class WinApiWindow;

	struct ApplicationCreateInfo
	{
		String Title = String(PROJECT_NAME);
		vath::Rect<u32> Rect = vath::Rect<u32>(0, 0, 1600, 900);
	};

	class DxrTracer final
	{
	public:
		DxrTracer(const ApplicationCreateInfo& a_info);
		~DxrTracer() = default;

	private:
		void EngineLoop();

		std::unique_ptr<WinApiWindow> m_window;
	};
}