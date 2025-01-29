#include "riow/traceable/sphere.h"
#include "riow/camera.h"
#include "riow/renderer.h"
#include "riow/material.h"
#include "riow/texture.h"
#include "riow/image.h"

using namespace dxray;

/// <summary>
/// Type defined to identify scenes.
/// </summary>
enum class EScene : u8
{
	BouncingSpheres = 0,
	PerlinSpheres
};

void BuildBouncingSpheresSceneComposition(riow::Camera& a_camera, riow::Scene& a_scene)
{
	//Camera.
	a_camera.SetVerticalFov(vath::DegToRad(20.0f));
	a_camera.SetAperture(0.35f);
	a_camera.SetFocalLength(10.0f);
	a_camera.SetShutterSpeed(0.001f);
	a_camera.LookAt(vath::Vector3f(13.0f, 2.0f, 3.0f), vath::Vector3f(0.0f, 0.0f, 0.0f));

	//Scene.
	std::shared_ptr<riow::CheckerBoard> checkerboardTex = std::make_shared<riow::CheckerBoard>(0.32f, riow::Color(0.1f), riow::Color(0.9f));
	std::shared_ptr<riow::Lambertian> groundMat = std::make_shared<riow::Lambertian>(checkerboardTex);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -1000.0f, 0.0f), 1000.0f, groundMat));
    std::shared_ptr<riow::ImageTexture> moonTexture = std::make_shared<riow::ImageTexture>(riow::Image::LoadFromFile(riow::AssetRootDirectory / "textures/diffuseMoon.jpg", riow::Image::ELoadOptions::FlipVertically, 3));

	for (i32 i = -11; i < 11; i++)
	{
		for (i32 j = -11; j < 11; j++)
		{
			const fp32 randomMat = vath::RandomNumber<fp32>();
			const vath::Vector3f center(i + 0.9f * vath::RandomNumber<fp32>(), 0.2f, j + 0.9f * vath::RandomNumber<fp32>());
			
			if (vath::Magnitude(center - vath::Vector3f(4.0f, 0.2f, 0.0f)) > 0.9f)
			{
				std::shared_ptr<riow::Material> sphereMat;
                const vath::Vector3f translation = center + vath::Vector3f(0.0f, vath::RandomNumber<fp32>(0.0f, 1.0f), 0.0f);

				if (randomMat < 0.25f)
				{
					//5% chance on moon
                    if (randomMat < 0.05f)
                    {
                        //Emissive moon lights.
                        sphereMat = std::make_shared<riow::DiffuseLight>(moonTexture, 1.0f);
                        a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, translation, 0.2f, sphereMat));
						continue;
                    }

					//20% - Emissive - emitting light.
                    const riow::Color albedo(vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>());
                    sphereMat = std::make_shared<riow::DiffuseLight>(albedo, vath::RandomNumber<fp32>(0.5f, 1.0));
                    a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, translation, 0.2f, sphereMat));
					continue;
				}
				else if (randomMat < 0.8f)
				{
                    //55% - Lambertian.
					sphereMat = std::make_shared<riow::Lambertian>(riow::Color(vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>()));
                    a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, translation, 0.2f, sphereMat));
					continue;
				}
				else if (randomMat < 0.95f)
				{
					//15% - Metallic.
					const riow::Color metallic(vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>(), vath::RandomNumber<fp32>());
					const fp32 fuzzy = vath::RandomNumber<fp32>();
					sphereMat = std::make_shared<riow::Metallic>(metallic, fuzzy);
                    a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, 0.2f, sphereMat));
					continue;
				}

				//5% - Dielectric.
				sphereMat = std::make_shared<riow::Dielectric>(1.5f);
                a_scene.AddTraceable(std::make_shared<riow::Sphere>(center, 0.2f, sphereMat));
			}
		}
	}

	std::shared_ptr<riow::Dielectric> largeDielectric = std::make_shared<riow::Dielectric>(1.5f);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0, 1, 0), 1.0f, largeDielectric));

	std::shared_ptr<riow::DiffuseLight> largeLambert = std::make_shared<riow::DiffuseLight>(riow::Color(0.4f, 0.2f, 0.1f));
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(-4, 1, 0), 1.0f, largeLambert));

	std::shared_ptr<riow::Metallic> largeMetal = std::make_shared<riow::Metallic>(riow::Color(0.4f, 0.2f, 0.1f), 0.0f);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(4, 1, 0), 1.0f, largeMetal));
}

void BuildPerlinSphereSceneComposition(riow::Camera& a_camera, riow::Scene& a_scene)
{
    //Camera.
    a_camera.SetVerticalFov(vath::DegToRad(20.0f));
    a_camera.SetAperture(0.001f);
    a_camera.SetFocalLength(10.0f);
    a_camera.SetShutterSpeed(0.001f);
    a_camera.LookAt(vath::Vector3f(12.0f, 2.0f, 3.0f), vath::Vector3f(0.0f, 0.0f, 0.0f));

	//Scene.
	std::shared_ptr<riow::NoiseTexture> noiseTex = std::make_shared<riow::NoiseTexture>(4.0f);
	std::shared_ptr<riow::Lambertian> lambertian = std::make_shared<riow::Lambertian>(noiseTex);
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, -1000.0f, 0.0f), 1000.0f, lambertian));
	a_scene.AddTraceable(std::make_shared<riow::Sphere>(vath::Vector3f(0.0f, 2.0f, 0.0f), 2.0f, lambertian));
}

int main(int argc, char** argv)
{
    DXRAY_INFO("=================================");
    DXRAY_INFO("= CPU RAYTRACER =");
    DXRAY_INFO("=================================\n");

	Stopwatchf timer;
	timer.Start();

	//STB expects signed integers for image writing, the values will never be negative regardless within the framework, so use unsigned integers onwards.
	const vath::Vector2i32 imageDimensions(2560, 1440);
	const i32 imageChannelNum = 3;
	const u32 clusterSize = 4;
	DXRAY_ASSERT_WITH_MSG(imageDimensions.x % clusterSize == 0, "Image width should be divisible by the cluster size. Clamping is currently not implemented.");
    DXRAY_ASSERT_WITH_MSG(imageDimensions.y % clusterSize == 0, "Image height should be divisible by the cluster size. Clamping is currently not implemented.");

	riow::Camera camera;
	camera.SetViewportDimensionInPx(vath::Vector2u32(imageDimensions.x, imageDimensions.y));

	//#Todo: can potentially make the selected scene be a commandline argument.
	const EScene selectedScene = EScene::BouncingSpheres;
	riow::Scene scene;
	switch (selectedScene)
	{
	case EScene::BouncingSpheres:
	{
		DXRAY_INFO("Scene: Bouncing spheres");
		BuildBouncingSpheresSceneComposition(camera, scene);
		break;
	}
	case EScene::PerlinSpheres:
    {
        DXRAY_INFO("Scene: Perlin spheres");
        BuildPerlinSphereSceneComposition(camera, scene);
        break;
	}
	default:
	{
		DXRAY_ERROR("Non-recognized scene.");
		return 1;//exit application.
	}
	}

	const riow::RendererPipeline renderPipeline =
	{
		.MaxTraceDepth = 100,
		.SuperSampleFactor = 4,
		.DepthOfFieldSampleCount = 16,
		.ClusterSize = clusterSize
	};

	riow::Renderer renderer;
	renderer.SetCamera(camera);
	renderer.SetBackgroundColor(riow::Color(0.01f));
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
	riow::SaveColorBufferToFile("riowOutput", riow::Image::EFileExtension::png, imageDimensions.x, imageDimensions.y, imageChannelNum, static_cast<riow::Color*>(imageData.data()), true);
	DXRAY_INFO("Saving results took {} ms.", timer.GetElapsedMs());
    DXRAY_INFO("=================================");
	
	return 0;
}