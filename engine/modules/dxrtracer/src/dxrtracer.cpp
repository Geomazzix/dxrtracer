#include "dxrtracer/shaderCompiler.h"
#include "dxrtracer/window.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/renderer.h"
#include "dxrtracer/renderpass.h"
#include "dxrtracer/camera.h"

#include <core/vath/vath.h>
#include <core/time/stopwatch.h>

using namespace dxray;

//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/box/glTF/box.gltf";
//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/waterbottle/glTF/WaterBottle.gltf";
const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/sponza/glTF/Sponza.gltf";

u32 m_windowSurfaceWidth = 1920;
u32 m_windowSurfaceHeight = 1080;

Stopwatchf m_appTime;
std::shared_ptr<WinApiWindow> m_window;
std::shared_ptr<Camera> m_camera;
std::unique_ptr<Renderer> m_renderer;

bool EngineInitialize()
{
	const WindowCreationInfo windowInfo =
	{
		.Title = PROJECT_NAME,
		.Rect = dxray::vath::Rectu32(0, 0, m_windowSurfaceWidth, m_windowSurfaceHeight)
	};
	m_window = std::make_shared<WinApiWindow>(windowInfo);
	
	m_camera = std::make_unique<Camera>(vath::DegToRad(45.0f), static_cast<fp32>(m_windowSurfaceWidth) / m_windowSurfaceHeight, 0.1f, 1000.0f);
	m_camera->LookAt(vath::Vector3f(6.5f, 4.0f, 0.0f), vath::Vector3f(4.0f, 2.5f, 0.0f), vath::Vector3f(0.0f, 1.0f, 0.0f));

	const RendererCreateInfo rendererInfo =
	{
		.MainCam = m_camera,
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
	m_renderer->LoadModel(vath::Vector3f(0.0f, 0.0f, 0.0f), vath::Vector3f(0.0f), vath::Vector3f(20.0f), quad);
	m_renderer->LoadModel(vath::Vector3f(-6.5f, 0.1f, 0.025f), vath::Vector3f(0.0f), vath::Vector3f(8.0f, 0.1f, 8.0f), quad);
	m_renderer->LoadModel(vath::Vector3f(0.0f), vath::Vector3f(0.0f), vath::Vector3f(0.008f), modelLoader.GetModel());
	m_renderer->EndResourceLoading();

	DXRAY_INFO("Initialization completed in: {}", m_appTime.GetElapsedSeconds());

	return true;
}

void EngineTick(const fp32 a_dt)
{
	m_camera->LookAt(vath::Vector3f(5.0f, 3.0f, 0.0f), vath::Vector3f(-4.0f, 3.5f + sinf(m_appTime.GetElapsedSeconds()), 0.0f));
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

		EngineTick(dt);
		m_renderer->Render(m_appTime.GetElapsedSeconds());

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