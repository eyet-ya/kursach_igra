#pragma once
#include <GL/gl.h>
#include <GL/glu.h>
#include "Texture.h"


struct Material {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
};

class Platform {
public:
    double x, y, z;
    double width, height, depth;
    bool hasTexture;
    Texture* texture;
    Material material;

    Platform();
    void Draw();
};
