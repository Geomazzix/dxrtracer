#include "dxrtracer/shaderCompiler.h"
#include "dxrtracer/window.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/scene.h"
#include "dxrtracer/renderer.h"
#include "dxrtracer/renderpass.h"

#include <core/vath/vath.h>
#include <core/time/stopwatch.h>

using namespace dxray;

//#Todo: no functions have proper error checking, add errors for different parts of the codebase.
//#Todo: Implement something alike vk::ArrayProxy. This would serve as a wrapper for std::array, std::vector and std::initializer_list which prevents raw pointer arguments.

u32 m_windowSurfaceWidth = 1920;
u32 m_windowSurfaceHeight = 1080;

//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/box/glTF/box.gltf";
const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/waterbottle/glTF/WaterBottle.gltf";
//const Path modelPath = Path(ENGINE_ROOT_DIRECTORY) / "samples/assets/models/sponza/glTF/Sponza.gltf";

std::shared_ptr<WinApiWindow> m_window;
std::shared_ptr<Scene> m_scene;
std::unique_ptr<Renderer> m_renderer;

int main(int argc, char** argv)
{
	Stopwatchf appTime;
	appTime.Start();

    const WindowCreationInfo windowInfo =
    {
        .Title = PROJECT_NAME,
        .Rect = dxray::vath::Rectu32(0, 0, m_windowSurfaceWidth, m_windowSurfaceHeight)
    };
    m_window = std::make_shared<WinApiWindow>(windowInfo);
	m_scene = std::make_shared<Scene>();
	
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
		return -1;
	}

	Model quad = BuildQuadModel();

	m_renderer->BeginResourceLoading();
	m_renderer->LoadModel(m_scene, quad);
	m_renderer->LoadModel(m_scene, quad);
	m_renderer->LoadModel(m_scene, modelLoader.GetModel());
	m_renderer->EndResourceLoading();

	fp32 elapsedInterval = 0.0f;
	fp32 prevFrameSample = 0.0f;
	u32 fps = 0;

	while (m_window->PollEvents())
	{
		const fp32 dt = appTime.GetElapsedSeconds() - prevFrameSample;
		prevFrameSample = appTime.GetElapsedSeconds();

		m_scene->Tick(dt);
		m_renderer->Render(m_scene);

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

	return 0;
}