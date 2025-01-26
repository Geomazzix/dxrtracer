#include "riow/traceable/sphere.h"
#include "riow/camera.h"
#include "riow/renderer.h"
#include "riow/material.h"

#define COMPOSITION_CLASSIC 0
#define COMPOSITION_FINAL 1

using namespace dxray;

void BuildClassicSphereSceneComposition(riow::Camera& a_camera, riow::Scene& a_scene)
{
	a_camera.SetVerticalFov(vath::DegToRad(20.0f));
	a_camera.SetAperture(1.0f);
	a_camera.SetFocalLength(3.4f);
	a_camera.LookAt(vath::Vector3f(-2.0f, 2.0f, 1.0f), vath::Vector3f(0.0f, 0.0f, -1.0f));

	std::shared_ptr<riow::Lambertian> ground = std::make_shared<riow::Lambertian>(riow::Color(0.8f, 0.8f, 0.0f));
	std::shared_ptr<riow::Lambertian> center = std::make_shared<riow::Lambertian>(riow::Color(0.1f, 0.2f, 0.5f));
	std::shared_ptr<riow::Dialectric> left = std::make_shared<riow::Dialectric>(1.5f);
	std::shared_ptr<riow::Dialectric> bubble = std::make_shared<riow::Dialectric>(1.0f / 1.5f);
	std::shared_ptr<riow::Metallic> right = std::make_shared<riow::Metallic>(riow::Color(0.8f, 0.6f, 0.2f), 1.0f);

	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -100.5f, -1.0f), 100.0f, ground));	//Ground
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, 0.0f, -1.2f), 0.5f, center));		//Middle sphere
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-1.0f, 0.0f, -1.0f), 0.5f, left));		//Left (inner sphere)
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-1.0f, 0.0f, -1.0f), 0.4f, bubble));		//bubble (outer sphere)
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(1.0f, 0.0f, -1.0f), 0.5f, right));		//Right
}

void BuildFinalSphereSceneComposition(riow::Camera& a_camera, riow::Scene& a_scene)
{
	//Camera.
	a_camera.SetVerticalFov(vath::DegToRad(20.0f));
	a_camera.SetAperture(0.25f);
	a_camera.SetFocalLength(10.0f);
	a_camera.LookAt(vath::Vector3f(13.0f, 2.0f, 3.0f), vath::Vector3f(0.0f, 0.0f, 0.0f));

	//Scene.
	std::shared_ptr<riow::Lambertian> groundMat = std::make_shared<riow::Lambertian>(riow::Color(0.5f));
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -1000.0f, 0.0f), 1000.0f, groundMat));

	for (i32 i = -11; i < 11; i++)
	{
		for (i32 j = -11; j < 11; j++)
		{
			const fp32 randomMat = vath::RandomNumber<fp32>();
			const vath::Vector3f center(i + 0.9f * vath::RandomNumber<fp32>(), 0.2f, j + 0.9f * vath::RandomNumber<fp32>());
			
			if (vath::Magnitude(center - vath::Vector3f(4.0f, 0.2f, 0.0f)) > 0.9f)
			{
				std::shared_ptr<riow::Material> sphereMat;

				if (randomMat < 0.8f)
				{
					const riow::Color albedo(vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>());
					sphereMat = std::make_shared<riow::Lambertian>(albedo);
				}
				else if (randomMat < 0.95f)
				{
					const riow::Color metallic(vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>());
					const fp32 fuzzy = vath::RandomNumber<fp32>();
					sphereMat = std::make_shared<riow::Metallic>(metallic, fuzzy);
				}
				else
				{
					sphereMat = std::make_shared<riow::Dialectric>(1.5f);
				}

				a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, 0.2f, sphereMat));
			}
		}
	}

	std::shared_ptr<riow::Dialectric> largeDialectric = std::make_shared<riow::Dialectric>(1.5f);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0, 1, 0), 1.0f, largeDialectric));

	std::shared_ptr<riow::Lambertian> largeLambert = std::make_shared<riow::Lambertian>(riow::Color(0.4f, 0.2f, 0.1f));
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-4, 1, 0), 1.0f, largeLambert));

	std::shared_ptr<riow::Metallic> largeMetal = std::make_shared<riow::Metallic>(riow::Color(0.4f, 0.2f, 0.1f), 0.0f);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(4, 1, 0), 1.0f, largeMetal));
}

int main(int argc, char** argv)
{
    DXRAY_INFO("=================================");
    DXRAY_INFO("= CPU RAYTRACER =");
    DXRAY_INFO("=================================\n");

	Stopwatchf timer;
	timer.Start();

	const vath::Vector2i32 imageDimensions(1600, 900);
	const i32 imageChannelNum = 3;

	riow::Camera camera;
	camera.SetViewportDimensionInPx(vath::Vector2u32(imageDimensions.x, imageDimensions.y));
	camera.SetZNear(0.001f);
	camera.SetZFar(1000.0f);
	riow::Scene scene;

#if COMPOSITION_CLASSIC
	BuildClassicSphereSceneComposition(camera, scene);
#elif COMPOSITION_FINAL
	BuildFinalSphereSceneComposition(camera, scene);
#endif

	const riow::RendererPipeline renderPipeline =
	{
		.MaxTraceDepth = 100,
		.SuperSampleFactor = 4,
		.DepthOfFieldSampleCount = 4
	};

	riow::Renderer renderer;
	renderer.SetCamera(camera);
	renderer.SetRenderPipeline(renderPipeline);

    DXRAY_INFO("=================================");
    DXRAY_INFO("Initializing resources.");
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