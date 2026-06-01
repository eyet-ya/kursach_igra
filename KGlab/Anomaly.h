#pragma once
#include <GL/gl.h>
#include <GL/glu.h>

class Anomaly {
public:
    double x, y, z;
    double radius;
    float color[4];
    bool visible;

    Anomaly();
    void Draw();
};