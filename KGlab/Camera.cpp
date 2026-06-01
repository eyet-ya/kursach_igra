#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Camera.h"
#include <cmath>

Camera::Camera() : camDist(5), camNz(1), camX(0), camY(0), camZ(0),
mouseX(-1), mouseY(-1), drag(false), _fi1(1), _fi2(0.5),
targetX(0), targetY(0), targetZ(0), offsetDist(4.0), offsetHeight(1.5)
{
    caclulateCameraPos();
}

void Camera::SetTarget(double x, double y, double z) {
    targetX = x;
    targetY = y;
    targetZ = z;
}

void Camera::UpdateRotation(double dx, double dy) {
    _fi1 += 0.008 * dx;   
    _fi2 -= 0.008 * dy;   

    if (_fi2 > 1.4) _fi2 = 1.4;
    if (_fi2 < -0.5) _fi2 = -0.5;
}

void Camera::FollowPlayer(double playerX, double playerY, double playerZ) {
    targetX = playerX;
    targetY = playerY;
    targetZ = playerZ;

    camDist = offsetDist;
    caclulateCameraPos();

    camX = targetX + camX;
    camY = targetY + camY;
    camZ = targetZ + camZ + offsetHeight; 
}

void Camera::setPosition(double x, double y, double z) {
    targetX = x;
    targetY = y;
    targetZ = z;
    camX = x;
    camY = y;
    camZ = z;
    camDist = sqrt(x * x + y * y + z * z);
    _fi1 = atan2(y, x);
    _fi2 = atan2(z, sqrt(x * x + y * y));
}

void Camera::caclulateCameraPos() {
    camX = camDist * cos(_fi2) * cos(_fi1);
    camY = camDist * cos(_fi2) * sin(_fi1);
    camZ = camDist * sin(_fi2);
    if (cos(_fi2) <= 0)
        camNz = -1;
    else
        camNz = 1;
}

void Camera::Zoom(OpenGL* sender, MouseWheelEventArg arg) {
    if (arg.value < 0 && offsetDist <= 2) return;
    if (arg.value > 0 && offsetDist >= 10) return;

    offsetDist += 0.05 * arg.value;
    camDist = offsetDist;
    caclulateCameraPos();
}

void Camera::MouseMovie(OpenGL* sender, MouseEventArg arg) {
    if (OpenGL::isKeyPressed('G')) return;

    if (mouseX == -1) {
        mouseX = arg.x;
        mouseY = arg.y;
        return;
    }
    int dx = mouseX - arg.x;
    int dy = mouseY - arg.y;
    mouseX = arg.x;
    mouseY = arg.y;

    if (drag) {
        UpdateRotation(dx, dy);
    }
}

void Camera::SetUpCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, targetX, targetY, targetZ, 0, 0, camNz);
}
