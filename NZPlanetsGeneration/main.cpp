#include <Nazara/Graphics.hpp>
#include <Nazara/Renderer.hpp>
#include <Nazara/Utility.hpp>
#include <NDK/Application.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <NDK/World.hpp>
#include <iostream>

#include "Generator.h"
#include "Render.h"

#include <random>
#include "SphericalDistribution.h"

int main()
{
	Nz::Clock c;
	/*PerlinData d(0);
	d.passCount = 2;
	d.passDivisor = 400;
	d.passPointMultiplier = 20;
	d.pointCount = 200;
	d.amplitude = 0.1f;
	SphereSurface<float> surface(perlin(d));
	surface.setRadius(3);*/

	/*for (auto it(surface.blocksBegin()); it != surface.blocksEnd(); it++)
		if (it->data < 0)
			it->data = -0.1;
		else it->data = 0.1;*/

	/*SphericalDistribution<float> pitchDistrib;
	std::uniform_real_distribution<float> yawDistrib(0, 2 * float(M_PI));
	std::mt19937 engine;

	SphereSurface<float> surface(3);

	for (unsigned int i(0); i < 2000; i++)
		surface.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)));

	surface.buildMap();
	SphereSurface<float> surface2(relax(surface));
	surface2 = relax(surface2);
	surface2 = relax(surface2);
	surface2 = relax(surface2);*/

	WorldMakerData d(1, 5000, 200);
	d.biomes.push_back(Biome(0, 0, Nz::Color(66, 134, 244), BiomeType::LAKE));
	d.biomes.push_back(Biome(0, 0, Nz::Color(44, 69, 170), BiomeType::OCEAN));
	d.biomes.push_back(Biome(0, 0, Nz::Color(175, 117, 75), BiomeType::GROUND));
	Planet surface(createWorld(d));
	surface.setRadius(3);

	std::cout << c.GetSeconds() << std::endl;

	Ndk::Application application;

	Nz::RenderWindow& mainWindow = application.AddWindow<Nz::RenderWindow>();
	mainWindow.Create(Nz::VideoMode(800, 600, 32), "Test", Nz::WindowStyle_Default, Nz::ContextParameters(Nz::RenderTargetParameters(1)));
	mainWindow.SetFramerateLimit(60);

	Ndk::World& world = application.AddWorld();
	world.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Up());

	Ndk::EntityHandle viewEntity = world.CreateEntity();
	viewEntity->AddComponent<Ndk::NodeComponent>();

	Ndk::CameraComponent& viewer = viewEntity->AddComponent<Ndk::CameraComponent>();
	viewer.SetTarget(&mainWindow);
	viewer.SetProjectionType(Nz::ProjectionType_Perspective);

	Nz::ModelRef model = render(surface);
	Ndk::EntityHandle planet = world.CreateEntity();
	Ndk::NodeComponent& planetNode = planet->AddComponent<Ndk::NodeComponent>();
	Ndk::GraphicsComponent& planetGraphics = planet->AddComponent<Ndk::GraphicsComponent>();
	planetGraphics.Attach(model);
	planetNode.SetPosition(0, 0, -10);

	/*Nz::ModelRef model2 = render(surface2);
	Ndk::EntityHandle planet2 = world.CreateEntity();
	Ndk::NodeComponent& planetNode2 = planet2->AddComponent<Ndk::NodeComponent>();
	Ndk::GraphicsComponent& planetGraphics2 = planet2->AddComponent<Ndk::GraphicsComponent>();
	planetGraphics2.Attach(model2);
	planetNode2.SetPosition(4, 0, -10);*/

	Ndk::EntityHandle light = world.CreateEntity();
	Ndk::NodeComponent& lightNode = light->AddComponent<Ndk::NodeComponent>();
	Ndk::LightComponent & lightLight = light->AddComponent<Ndk::LightComponent>();
	lightLight.SetLightType(Nz::LightType_Point);
	lightLight.SetAmbientFactor(0.1f);
	lightLight.SetRadius(100);
	lightNode.SetPosition(Nz::Vector3f(5, 5, 0));

	world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(117, 122, 214)));

	while (application.Run())
	{
		planetNode.SetRotation(Nz::Quaternionf(Nz::EulerAnglesf(c.GetSeconds()*10, c.GetSeconds()*13.2f, 0)));
		//planetNode2.SetRotation(Nz::Quaternionf(Nz::EulerAnglesf(c.GetSeconds() * 10, c.GetSeconds()*13.2f, 0)));
		mainWindow.Display();
	}

	return EXIT_SUCCESS;
}
