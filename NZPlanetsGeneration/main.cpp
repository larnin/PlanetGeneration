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
	PerlinData d(0);
	d.passCount = 5;
	d.passDivisor = 2;
	d.passPointMultiplier = 2;
	d.pointCount = 20;
	d.amplitude = 0.1f;
	SphereSurface<float> surface(perlin(d));
	surface.setRadius(3);

	std::cout << c.GetSeconds() << std::endl;

	Ndk::Application application;

	Nz::RenderWindow& mainWindow = application.AddWindow<Nz::RenderWindow>();
	mainWindow.Create(Nz::VideoMode(800, 600, 32), "Test", Nz::WindowStyle_Default, Nz::ContextParameters(Nz::RenderTargetParameters(1)));

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
		mainWindow.Display();
	}

	return EXIT_SUCCESS;
}