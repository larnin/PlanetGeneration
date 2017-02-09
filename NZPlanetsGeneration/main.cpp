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

	WorldMakerData d(6, 2.5f);
	d.biomes.push_back(Biome(0, 0, BiomeType::LAKE, RandomColor(Nz::Color(85, 125, 166))));
	d.biomes.push_back(Biome(0, 0, BiomeType::OCEAN, RandomColor(Nz::Color(54, 54, 97))));
	d.biomes.push_back(Biome(0.875f, 0.75f, BiomeType::GROUND, RandomColor(Nz::Color(248, 248, 248)))); //snow
	d.biomes.push_back(Biome(0.875f, 0.42f, BiomeType::GROUND, RandomColor(Nz::Color(221, 221, 187)))); //tundra
	d.biomes.push_back(Biome(0.875f, 0.25f, BiomeType::GROUND, RandomColor(Nz::Color(187, 187, 187)))); //bare
	d.biomes.push_back(Biome(0.875f, 0.08f, BiomeType::GROUND, RandomColor(Nz::Color(153, 153, 153)))); //scorched
	d.biomes.push_back(Biome(0.625f, 0.83f, BiomeType::GROUND, RandomColor(Nz::Color(204, 212, 187)))); //taiga
	d.biomes.push_back(Biome(0.625f, 0.50f, BiomeType::GROUND, RandomColor(Nz::Color(196, 204, 187)))); //shrubland
	d.biomes.push_back(Biome(0.625f, 0.17f, BiomeType::GROUND, RandomColor(Nz::Color(228, 232, 202)))); //temperate desert
	d.biomes.push_back(Biome(0.375f, 0.92f, BiomeType::GROUND, RandomColor(Nz::Color(164, 196, 168)))); //teperate rain forest
	d.biomes.push_back(Biome(0.375f, 0.67f, BiomeType::GROUND, RandomColor(Nz::Color(180, 201, 169)))); //temperate deciduous forest
	d.biomes.push_back(Biome(0.375f, 0.33f, BiomeType::GROUND, RandomColor(Nz::Color(196, 212, 170)))); //grassland
	d.biomes.push_back(Biome(0.375f, 0.08f, BiomeType::GROUND, RandomColor(Nz::Color(228, 232, 202)))); //temperate desert
	d.biomes.push_back(Biome(0.125f, 0.83f, BiomeType::GROUND, RandomColor(Nz::Color(156, 187, 169)))); //tropical rain forest
	d.biomes.push_back(Biome(0.125f, 0.50f, BiomeType::GROUND, RandomColor(Nz::Color(169, 204, 164)))); //tropical seasonial forest
	d.biomes.push_back(Biome(0.125f, 0.25f, BiomeType::GROUND, RandomColor(Nz::Color(196, 212, 170)))); //grassland
	d.biomes.push_back(Biome(0.125f, 0.08f, BiomeType::GROUND, RandomColor(Nz::Color(233, 221, 199)))); //subtropical desert
	d.groundScale = 2.0f;
	d.elevationNoiseScale = 15.0f;
	d.elevationNoiseAmplitude = 0.1f;
	d.haveWater = true;
	d.waterLevel = 0.3f;
	d.maxHeight = 0.3f;
	d.maxDepth = 0.15f;
	d.rivierCount = 50;
	d.elevationAmplification = 2.5f;
	d.waterDepthAmplification = 0.7f;
	Generator generator(d);
	Planet surface(generator.create(83));
	surface.setRadius(4);

	Ndk::Application application;
	//application.EnableFPSCounter(true);

	Nz::RenderWindow& mainWindow = application.AddWindow<Nz::RenderWindow>();
	mainWindow.Create(Nz::VideoMode(800, 600, 32), "Test", Nz::WindowStyle_Default , Nz::ContextParameters(Nz::RenderTargetParameters(1)));
	mainWindow.SetFramerateLimit(60);

	Ndk::World& world = application.AddWorld();
	world.GetSystem<Ndk::RenderSystem>().SetGlobalUp(Nz::Vector3f::Up());

	Ndk::EntityHandle viewEntity = world.CreateEntity();
	Ndk::NodeComponent& cameraNode = viewEntity->AddComponent<Ndk::NodeComponent>();

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
	planetNode2.SetPosition(-1, 0, -10);*/

	Ndk::EntityHandle light = world.CreateEntity();
	Ndk::NodeComponent& lightNode = light->AddComponent<Ndk::NodeComponent>();
	Ndk::LightComponent & lightLight = light->AddComponent<Ndk::LightComponent>();
	lightLight.SetLightType(Nz::LightType_Point);
	lightLight.SetAmbientFactor(0.1f);
	lightLight.SetRadius(100);
	lightNode.SetPosition(Nz::Vector3f(5, 5, 5));

	world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(117, 122, 214)));

	std::cout << "Ready : " << c.GetSeconds() << std::endl;

	auto & eventHandler(mainWindow.GetEventHandler());
	eventHandler.OnMouseMoved.Connect([&planetNode](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent & event)
	{
		Nz::EulerAnglesf euler(planetNode.GetRotation().ToEulerAngles());
		euler.yaw += event.deltaX / 2.0f;
		euler.pitch += event.deltaY / 2.0f;
		euler.roll = 0;
		if (euler.pitch > 90)
			euler.pitch = 90;
		if (euler.pitch < -90)
			euler.pitch = -90;

		planetNode.SetRotation(Nz::Quaternionf(euler));
	});

	eventHandler.OnMouseWheelMoved.Connect([&cameraNode](const Nz::EventHandler*, const Nz::WindowEvent::MouseWheelEvent & event)
	{
		cameraNode.Move(0, 0, event.delta);
	});

	eventHandler.OnKeyPressed.Connect([&mainWindow](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent & event)
	{
		if (event.code == Nz::Keyboard::Key::Escape)
			mainWindow.Close();
	});

	while (application.Run())
	{
		//planetNode.SetRotation(Nz::Quaternionf(Nz::EulerAnglesf(c.GetSeconds()*10, c.GetSeconds()*13.2f, 0)));
		//planetNode2.SetRotation(Nz::Quaternionf(Nz::EulerAnglesf(c.GetSeconds() * 10, c.GetSeconds()*13.2f, 0)));
		mainWindow.Display();
	}

	return EXIT_SUCCESS;
}


