#include "dxrtracer/dxrtracer.h"
#include "dxrtracer/window.h"
#include "dxrtracer/renderer.h"
#include <core/time/stopwatch.h>

namespace dxray
{
	Stopwatchf DxrTracer::DxrTracer::s_appTime;

	DxrTracer::DxrTracer(const ApplicationCreateInfo& a_info)
	{
        s_appTime.Start();

		const WindowCreationInfo windowInfo =
		{
			.Title = a_info.Title,
			.Rect = a_info.Rect
		};

		m_window = std::make_unique<WinApiWindow>(windowInfo);

		const RendererCreateInfo rendererInfo =
		{
			.WindowRect = vath::Rectu32(0u, 0u, 1600u, 900u),
			.WindowHandle = m_window->GetNativeHandle(),
			.bUseWarp = false
		};
		
		m_renderer = std::make_unique<Renderer>(rendererInfo);

		EngineLoop();
	}

	void DxrTracer::EngineLoop()
	{
		fp32 elapsedInterval = 0.0f;
		u64 fps = 0u;

		while (m_window->PollEvents())
		{
			//m_renderer->Render();

			++fps;
			if (s_appTime.GetElapsedSeconds() - elapsedInterval > 1.0f)
			{
				m_window->SetWindowTitle(std::format("{} fps: {} - mspf: {}", 
					PROJECT_NAME, 
					static_cast<fp32>(fps), 
					static_cast<fp32>(1000.0f / fps))
				);

				elapsedInterval += 1.0f;
				fps = 0;
			}
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