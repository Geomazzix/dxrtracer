#include "dxrtracer/dxrtracer.h"
#include "dxrtracer/window.h"

namespace dxray
{
	DxrTracer::DxrTracer(const ApplicationCreateInfo& a_info)
	{
		const WindowCreationInfo windowInfo =
		{
			.Title = a_info.Title,
			.Rect = a_info.Rect
		};

		m_window = std::make_unique<WinApiWindow>(windowInfo);

		EngineLoop();
	}

	void DxrTracer::EngineLoop()
	{
		u64 framecounter = 0;
		while (m_window->PollEvents())
		{
			m_window->SetWindowTitle("dxrtracer - framecount: " + std::to_string(framecounter));

			DXRAY_TRACE("IsMaximized: {}", m_window->IsMaximized());
			DXRAY_TRACE("IsMinimized: {}", m_window->IsMinimized());
			DXRAY_TRACE("GetDimensions: {}, {}", m_window->GetWidthInPx(), m_window->GetHeightInPx());

			framecounter++;
		}
	}
}

int main(int argc, char** argv)
{
	const dxray::ApplicationCreateInfo appInfo =
	{
		.Title = PROJECT_NAME,
		.Rect = dxray::vath::Rectu32(0, 0, 1600, 900)
	};

	dxray::DxrTracer m_application(appInfo);
	return 0;
}