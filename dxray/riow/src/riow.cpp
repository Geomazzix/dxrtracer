#include "riow/traceable/sphere.h"
#include "riow/camera.h"
#include "riow/renderer.h"

using namespace dxray;

int main(int argc, char** argv)
{
	Stopwatchf timer;
	timer.Start();

	//Aspect ratio and output image dimensions.
	const vath::Vector2i32 imageDimensions(1600, 900);
	const i32 imageChannelNum = 3;

	//Camera/viewport
	riow::Camera camera;
	camera.SetViewportDimensionInPx(vath::Vector2u32(imageDimensions.x, imageDimensions.y));
	camera.SetZNear(0.01f);
	camera.SetZFar(1000.0f);
	camera.SetFocalLength(1.0f);

	//Scene
	riow::Scene scene;
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -100.5f, -1.0f), 100.0f));	//Ground
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, 0.0f, -1.2f), 0.5f));		//Middle sphere

	//Renderer
	riow::RendererPipeline renderPipeline =
	{
		.AASampleCount = 4
	};

	riow::Renderer renderer;
	renderer.SetCamera(camera);
	renderer.SetRenderPipeline(renderPipeline);

	//Render the scene and store the result.
	std::vector<vath::Vector3> imageData(imageDimensions.x * imageDimensions.y);
	DXRAY_INFO("Initialization complete after {} ms.", timer.GetElapsedMs());

	timer.Reset();
	renderer.Render(scene, imageData);
	DXRAY_INFO("Rendering complete after {} ms.", timer.GetElapsedMs());

	timer.Reset();
	StorePNG("riowOutput", imageDimensions.x, imageDimensions.y, imageChannelNum, static_cast<vath::Vector3f*>(imageData.data()), true);
	DXRAY_INFO("Saved results to binary directory in {} ms.", timer.GetElapsedMs());
	
	return 0;
}