/*#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
*/

#include "sphericaldistribution.h"
#include "spheresurface.h"
#include <array>
#include <iostream>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include "render.h"

#include "utilities.h"

#include "generator.h"

const unsigned int nbPoints(55);

int main()
{
    SphericalDistribution<float> pitchDistrib;
    std::uniform_real_distribution<float> yawDistrib(0, 2*PI);
    std::default_random_engine e(11);

    sf::Clock c;
    /*SphereSurface<BlockInfo> surface(225);
    for(unsigned int i(0) ; i < nbPoints ; i++)
        surface.addBlock(SpherePoint(yawDistrib(e), pitchDistrib(e)));
    for(unsigned int i(0) ; i < 5 ; i++)
        surface = relaxation(surface, 3);
    surface.buildMap();*/
    PerlinData d(0);
    d.passCount = 5;
    d.passDivisor = 1.5;
    d.passPointMultiplier = 2;
    d.pointCount = nbPoints;
    d.amplitude = 20;
    SphereSurface<float> surface(perlin(d));
    surface.setRadius(225);;
    std::cout << c.getElapsedTime().asSeconds() << std::endl;
    //std::cout << std::distance(surface.trianglesBegin(), surface.trianglesEnd()) << std::endl;

    sf::RenderWindow window(sf::VideoMode(800, 800), "Sphere test");
    auto view = window.getView();
    view.setCenter(0, 0);
    window.setView(view);
    window.setVerticalSyncEnabled(true);


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::KeyPressed)
                surface = relaxation(surface, 3);
        }
        /*if(c.getElapsedTime().asSeconds() > 0.1f)
        {
            surface = relaxation(surface, 2);
            c.restart();
        }*/

        window.clear(sf::Color::Black);
        window.draw(drawTrianglesAndSides(surface, c.getElapsedTime().asSeconds()/12, c.getElapsedTime().asSeconds()/3, sf::Color::Red, sf::Color::Green));
        window.draw(drawBlocks(surface, c.getElapsedTime().asSeconds()/12, c.getElapsedTime().asSeconds()/3, sf::Color::White, sf::Color::Cyan));
        //window.draw(drawBlocks(surface, 0, 0, sf::Color::White));
        window.display();
    }

    return 0;
}
