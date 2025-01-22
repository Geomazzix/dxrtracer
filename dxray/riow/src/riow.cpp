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
	camera.SetZNear(0.001f);
	camera.SetZFar(1000.0f);
	camera.SetFocalLength(1.0f);

	//Scene
	riow::Scene scene;

	std::shared_ptr<riow::Lambertian> ground = std::make_shared<riow::Lambertian>(riow::Color(0.8f, 0.8f, 0.0f));
	std::shared_ptr<riow::Lambertian> center = std::make_shared<riow::Lambertian>(riow::Color(0.1f, 0.2f, 0.5f));
	std::shared_ptr<riow::Metalic> left = std::make_shared<riow::Metalic>(riow::Color(0.8f, 0.8f, 0.8f), 0.3f);
	std::shared_ptr<riow::Metalic> right = std::make_shared<riow::Metalic>(riow::Color(0.8f, 0.6f, 0.2f), 1.0f);

	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -100.5f, -1.0f), 100.0f, ground));	//Ground
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, 0.0f, -1.2f), 0.5f, center));		//Middle sphere
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-1.0f, 0.0f, -1.0f), 0.5f, left));			//Left
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(1.0f, 0.0f, -1.0f), 0.5f, right));			//Right

	//Renderer
	riow::RendererPipeline renderPipeline =
	{
		.MaxTraceDepth = 10,
		.AASampleCount = 4,
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