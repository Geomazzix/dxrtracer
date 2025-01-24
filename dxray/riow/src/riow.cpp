#include "riow/traceable/sphere.h"
#include "riow/camera.h"
#include "riow/renderer.h"

using namespace dxray;

int main(int argc, char** argv)
{
    DXRAY_INFO("=================================");
    DXRAY_INFO("= CPU RAYTRACER =");
    DXRAY_INFO("=================================\n");

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
	camera.SetVerticalFov(vath::DegToRad(90.0f));
	camera.LookAt(vath::Vector3f(-1.0f, 1.0f, 1.0f), vath::Vector3f(0.0f, 0.0f, -1.0f));

	//Scene
	riow::Scene scene;

	std::shared_ptr<riow::Lambertian> ground = std::make_shared<riow::Lambertian>(riow::Color(0.8f, 0.8f, 0.0f));
	std::shared_ptr<riow::Lambertian> center = std::make_shared<riow::Lambertian>(riow::Color(0.1f, 0.2f, 0.5f));
	std::shared_ptr<riow::Dialectric> left = std::make_shared<riow::Dialectric>(1.5f);
	std::shared_ptr<riow::Dialectric> bubble = std::make_shared<riow::Dialectric>(1.0f / 1.5f);
	std::shared_ptr<riow::Metalic> right = std::make_shared<riow::Metalic>(riow::Color(0.8f, 0.6f, 0.2f), 0.3f);
	
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -100.5f, -1.0f), 100.0f, ground));	//Ground
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, 0.0f, -1.2f), 0.5f, center));		//Middle sphere
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-1.0f, 0.0f, -1.0f), 0.5f, left));			//Left (inner sphere)
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-1.0f, 0.0f, -1.0f), 0.4f, bubble));		//bubble (outer sphere)
	scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(1.0f, 0.0f, -1.0f), 0.5f, right));			//Right

	//Renderer
	riow::RendererPipeline renderPipeline =
	{
		.MaxTraceDepth = 100, //In simple scenes this will mostly affect dialectics.
		.AASampleCount = 4,
	};

	riow::Renderer renderer;
	renderer.SetCamera(camera);
	renderer.SetRenderPipeline(renderPipeline);
    DXRAY_INFO("=================================");
    DXRAY_INFO("Initializing resources.");
	//Render the scene and store the result.
	std::vector<vath::Vector3> imageData(imageDimensions.x * imageDimensions.y);
	DXRAY_INFO("Init took {} ms.", timer.GetElapsedMs());
    DXRAY_INFO("=================================\n");

	timer.Reset();
    DXRAY_INFO("=================================");
    DXRAY_INFO("Starting render.");
	renderer.Render(scene, imageData);
	DXRAY_INFO("Rendering took {} ms.", timer.GetElapsedMs());
    DXRAY_INFO("=================================\n");

    DXRAY_INFO("=================================");
    DXRAY_INFO("Storing results to file...");
	timer.Reset();
	StorePNG("riowOutput", imageDimensions.x, imageDimensions.y, imageChannelNum, static_cast<vath::Vector3f*>(imageData.data()), true);
	DXRAY_INFO("Saving results took {} ms.", timer.GetElapsedMs());
    DXRAY_INFO("=================================");
	
	return 0;
}