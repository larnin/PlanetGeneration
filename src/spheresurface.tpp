//#include "spheresurface.h"
#include <cmath>
#include <SFML/System/Vector3.hpp>
#include <vectconvert.h>
#include "utilities.h"

template <typename T>
SphereSurface<T>::SphereSurface(float radius)
    : m_radius(radius)
    , m_builded(false)
{

}

template <typename T>
void SphereSurface<T>::addBlock(const SpherePoint & pos)
{
    m_blocks.emplace_back(pos);
}

template <typename T>
void SphereSurface<T>::buildMap()
{
    if(m_builded)
        return;

    m_borders.clear();
    m_points.clear();
    m_triangles.clear();

    for(auto it(m_blocks.begin()) ; it != m_blocks.end() ; it++)
    {
        sf::Vector3f pos1(toVector3(it->pos));
        for(auto it2(std::next(it)) ; it2 != m_blocks.end() ; it2++)
        {
            sf::Vector3f pos2(toVector3(it2->pos));
            for(auto it3(std::next(it2)) ; it3 != m_blocks.end() ; it3++)
            {
                sf::Vector3f pos3(toVector3(it3->pos));
                sf::Vector3f center(triangleOmega(pos1,pos2,pos3));
                float sqrRadius(sqrNorm(center-pos1));
                bool isOn = false;
                for(auto it4(m_blocks.begin()); it4 != m_blocks.end() ; it4++)
                {
                    if(it4 == it || it4 == it2 || it4 == it3)
                        continue;
                    if(sqrNorm(toVector3(it4->pos) - center) <= sqrRadius)
                    {
                        isOn = true;
                        break;
                    }
                }
                if(isOn)
                    continue;

                unsigned int b1(std::distance(m_blocks.begin(), it));
                unsigned int b2(std::distance(m_blocks.begin(), it2));
                unsigned int b3(std::distance(m_blocks.begin(), it3));

                Triangle t(b1, b2, b3);
                //if(std::find(m_triangles.begin(), m_triangles.end(), t) == m_triangles.end())
                {
                    m_triangles.push_back(t);
                    m_points.emplace_back(toSpherePoint(center));
                    Point p = m_points.back();
                    unsigned int pID = m_points.size()-1;
                    it->points.push_back(pID);
                    it2->points.push_back(pID);
                    it3->points.push_back(pID);
                    p.blocks.push_back(b1);
                    p.blocks.push_back(b2);
                    p.blocks.push_back(b3);
                }
            }
        }
    }
}

template <typename T>
SphereSurface<T> relaxation(const SphereSurface<T> & m, float diviser)
{
    SphereSurface<T> s(m.radius());
    for(auto it(m.blocksBegin()) ; it != m.blocksEnd() ; it++)
    {
        sf::Vector3f pos(toVector3(it->pos, s.radius()));
        for(auto it2(m.blocksBegin()) ; it2 != m.blocksEnd() ; it2++)
        {
            if(it == it2)
                continue;
            sf::Vector3f pos2(toVector3(it2->pos, s.radius()));
            pos -= pos2/(norm(pos2-pos)*diviser);
        }
        s.addBlock(toSpherePoint(pos));
    }

    return s;
}

