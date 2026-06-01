#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Player.h"
#include "Render.h"
#include <cmath>

Player::Player() : x(0), y(0), z(-0.1), vx(0), vy(0), vz(0), radius(0.4), onGround(false), yaw(0), animationTime(0), bodyAngle(0) {}

void Player::LoadModel() {
    model.LoadModel("models/player.obj");

    model.LoadAndSetTexture("body", "textures/body.png");
    model.LoadAndSetTexture("head", "textures/body.png");
    model.LoadAndSetTexture("ears", "textures/ears.png");
    model.LoadAndSetTexture("eyes", "textures/eyes.png");
    model.LoadAndSetTexture("nose", "textures/nose.png");
    model.LoadAndSetTexture("legs", "textures/body.png");
    model.LoadAndSetTexture("tail", "textures/body.png");

    model.BuildDisplayList();
}

void Player::SetYaw(double angle) {
    yaw = angle;
}

void Player::Jump() {
    if (onGround) {
        vz = 6.0;
        onGround = false;
    }
}

void Player::Update(double deltaTime, const std::vector<SimplePlatform>& platforms) {
    vz -= 12.0 * deltaTime;

    double oldX = x;
    double oldY = y;
    double oldZ = z;

    double newX = x + vx * deltaTime;
    x = newX;

    for (const auto& plat : platforms) {
        double left = plat.x - plat.sizeX / 2;
        double right = plat.x + plat.sizeX / 2;
        double bottom = plat.y - plat.sizeY / 2;
        double top = plat.y + plat.sizeY / 2;
        double platBottom = plat.z - plat.sizeZ / 2;
        double platTop = plat.z + plat.sizeZ / 2;

        if (x + radius > left && x - radius < right &&
            y + radius > bottom && y - radius < top &&
            z + radius > platBottom && z - radius < platTop) {
            x = oldX;
            break;
        }
    }

    double newY = y + vy * deltaTime;
    y = newY;

    for (const auto& plat : platforms) {
        double left = plat.x - plat.sizeX / 2;
        double right = plat.x + plat.sizeX / 2;
        double bottom = plat.y - plat.sizeY / 2;
        double top = plat.y + plat.sizeY / 2;
        double platBottom = plat.z - plat.sizeZ / 2;
        double platTop = plat.z + plat.sizeZ / 2;

        if (x + radius > left && x - radius < right &&
            y + radius > bottom && y - radius < top &&
            z + radius > platBottom && z - radius < platTop) {
            y = oldY;
            break;
        }
    }

    double newZ = z + vz * deltaTime;
    z = newZ;

    onGround = false;

    for (const auto& plat : platforms) {
        double left = plat.x - plat.sizeX / 2;
        double right = plat.x + plat.sizeX / 2;
        double bottom = plat.y - plat.sizeY / 2;
        double top = plat.y + plat.sizeY / 2;
        double platBottom = plat.z - plat.sizeZ / 2;
        double platTop = plat.z + plat.sizeZ / 2;

        if (x + radius > left && x - radius < right &&
            y + radius > bottom && y - radius < top) {

            if (vz <= 0 && z - radius <= platTop && oldZ - radius >= platTop - 0.3) {
                z = platTop + radius;
                vz = 0;
                onGround = true;
            }
            else if (vz >= 0 && z + radius >= platBottom && oldZ + radius <= platBottom + 0.3) {
                z = platBottom - radius;
                vz = 0;
            }
            else if (z + radius > platBottom && z - radius < platTop) {
                if (abs(z - platTop) < abs(z - platBottom)) {
                    z = platTop + radius;
                }
                else {
                    z = platBottom - radius;
                }
                vz = 0;
            }
        }
    }

    if (fabs(vx) > 0.1 || fabs(vy) > 0.1) {
        animationTime += deltaTime * 8;
        bodyAngle = sin(animationTime) * 0.4;
    }
    else {
        bodyAngle = 0;
    }

    if (onGround) {
        vx *= 0.96;
        vy *= 0.96;
    }

    if (z < -3.0) {
        x = 0;
        y = 0;
        z = -0.1;
        vx = 0;
        vy = 0;
        vz = 0;
        onGround = true;
    }
}

void Player::Draw() {
    glPushMatrix();
    glTranslated(x, y, z);
    glRotated(90, 1, 0, 0);
    glRotated(yaw * 180 / 3.14159, 0, 1, 0);
    glRotated(-90, 0, 1, 0);
    glTranslated(0, -0.4, 0);
    glScaled(0.5, 0.5, 0.5);

    Shader::DontUseShaders();
    glEnable(GL_TEXTURE_2D);

    model.Draw();

    glPopMatrix();
}