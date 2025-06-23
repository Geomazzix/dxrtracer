#include "dxrtracer/shaderCompiler.h"
#include "dxrtracer/window.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/renderer.h"
#include "dxrtracer/renderpass.h"

#include <core/vath/vath.h>
#include <core/time/stopwatch.h>

using namespace dxray;

//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/box/glTF/box.gltf";
const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/waterbottle/glTF/WaterBottle.gltf";
//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/sponza/glTF/Sponza.gltf";

u32 m_windowSurfaceWidth = 1920;
u32 m_windowSurfaceHeight = 1080;

Stopwatchf m_appTime;
std::shared_ptr<WinApiWindow> m_window;
std::unique_ptr<Renderer> m_renderer;

bool EngineInitialize()
{
	const WindowCreationInfo windowInfo =
	{
		.Title = PROJECT_NAME,
		.Rect = dxray::vath::Rectu32(0, 0, m_windowSurfaceWidth, m_windowSurfaceHeight)
	};
	m_window = std::make_shared<WinApiWindow>(windowInfo);

	const RendererCreateInfo rendererInfo =
	{
		.SwapchainInfo =
		{
			.SurfaceWidth = m_windowSurfaceWidth,
			.SurfaceHeight = m_windowSurfaceHeight,
			.Window = m_window
		}
	};
	m_renderer = std::make_unique<Renderer>(rendererInfo);

	AssimpModelLoader modelLoader(modelPath);
	if (!modelLoader.LoadModel())
	{
		return false;
	}

	const Model& quad = BuildQuadModel();

	m_renderer->BeginResourceLoading();
	m_renderer->LoadModel(vath::Vector3f(0.0f, 2.0f, 0.0f), vath::Vector3f(0.0f), vath::Vector3f(2.0f), modelLoader.GetModel());
	m_renderer->LoadModel(vath::Vector3f(0.0f, 2.0f, 1.0f), vath::Vector3f(1.8f, 0.0f, 0.0f), vath::Vector3f(3.0f), quad);
	m_renderer->LoadModel(vath::Vector3f(0.0f, 0.0f, 0.0f), vath::Vector3f(0.0f), vath::Vector3f(20.0f), quad);
	m_renderer->EndResourceLoading();

	DXRAY_INFO("Initialization completed in: {}", m_appTime.GetElapsedSeconds());

	return true;
}

void EngineLoop()
{
	fp32 elapsedInterval = 0.0f;
	fp32 prevFrameSample = 0.0f;
	u32 fps = 0;

	while (m_window->PollEvents())
	{
		const fp32 dt = m_appTime.GetElapsedSeconds() - prevFrameSample;
		prevFrameSample = m_appTime.GetElapsedSeconds();

		m_renderer->Render(dt);

		elapsedInterval += dt;
		++fps;
		if (elapsedInterval > 1.0f)
		{
			m_window->SetWindowTitle(std::format("{} fps: {} - mspf: {}",
				PROJECT_NAME,
				fps,
				1000.0f / fps
			));

			elapsedInterval = 0.0f;
			fps = 0;
		}
	}

	DXRAY_INFO("App exit after: {}", m_appTime.GetElapsedSeconds());
}

int main(int argc, char** argv)
{
	m_appTime.Start();

	EngineInitialize();
	EngineLoop();

	return 0;
}