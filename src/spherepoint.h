#ifndef SPHEREPOINT_H
#define SPHEREPOINT_H


struct SpherePoint
{
    SpherePoint(float _yaw, float _pitch)
        : yaw(_yaw), pitch(_pitch) {}

    float yaw; //[0, 2*PI] horizontal
    float pitch; //[0, PI] vertical
};

#endif // SPHEREPOINT_H
