#include "generator.h"
#include "sphericaldistribution.h"
#include <random>
#include <vector>

#include <iostream>

SphereSurface<float> perlinPass(auto engine, float max, unsigned int pointCount)
{
    std::uniform_real_distribution<float> d(-max, max);
    SphericalDistribution<float> pitchDistrib;
    std::uniform_real_distribution<float> yawDistrib(0, 2*PI);

    SphereSurface<float> s;

    for(unsigned int i(0) ; i < pointCount ; i++)
        s.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)));

    for(auto it(s.blocksBegin()) ; it != s.blocksEnd() ; it++)
        it->data = d(engine);
    relaxation(s, 3);
    s.buildMap();
    return s;
}

SphereSurface<float> perlin(const PerlinData & data)
{
    std::default_random_engine engine(data.seed);
    if(data.passCount == 0)
        return SphereSurface<float>();

    std::vector<SphereSurface<float>> surfaces;
    unsigned int pointCount(data.pointCount);
    float value(data.amplitude);
    for(unsigned int i(0) ; i < data.passCount ; i++)
    {
        surfaces.push_back(perlinPass(engine, value, pointCount));
        std::cout << "pass " << i << " done / " << pointCount << " points" << std::endl;
        value /= data.passDivisor;
        pointCount *= data.passPointMultiplier;
    }

    std::cout << "Stack pass ..." << std::endl;

    SphereSurface<float> s(surfaces.back());
    surfaces.pop_back();
    unsigned int index(0);
    for(const auto & surface : surfaces)
    {
        for(auto it(surface.trianglesBegin()) ; it != surface.trianglesEnd() ; it++)
        {
            const Block<float> & b1(*std::next(surface.blocksBegin(), it->block1));
            const Block<float> & b2(*std::next(surface.blocksBegin(), it->block2));
            const Block<float> & b3(*std::next(surface.blocksBegin(), it->block3));

            sf::Vector3f pos1(toVector3(b1.pos));
            sf::Vector3f pos2(toVector3(b2.pos));
            sf::Vector3f pos3(toVector3(b3.pos));

            for(auto it2(s.blocksBegin()) ; it2 != s.blocksEnd() ; it2++)
            {
                sf::Vector3f pos(toVector3(it2->pos));
                auto i(intersect(pos1, pos2, pos3, sf::Vector3f(0, 0, 0), pos));
                if(!i.first)
                    continue;

                sf::Vector3f p(proportions(pos1, pos2, pos3, i.second));
                it2->data += p.x*b1.data + p.y*b2.data + p.z*b3.data;
            }
        }
        std::cout << index++ << " stacked"<< std::endl;
    }
    std::cout << "end" << std::endl;
    return s;
}
