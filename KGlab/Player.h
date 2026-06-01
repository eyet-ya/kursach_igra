#pragma once
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include "Texture.h"
#include "ObjLoader.h"

class Player {
public:
    double x, y, z;
    double vx, vy, vz;
    double radius;
    bool onGround;
    double yaw;

    float animationTime;
    float bodyAngle;

    ObjModel model;
    Texture playerTexture;  

    Player();
    void Update(double deltaTime, const std::vector<struct SimplePlatform>& platforms);
    void Draw();
    void Jump();
    void Move(double dx, double dz);
    void SetYaw(double angle);
    void LoadModel();
    void LoadTexture();
};