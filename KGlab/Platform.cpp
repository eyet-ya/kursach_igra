#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Platform.h"

Platform::Platform() : x(0), y(0), z(0), width(1.0), height(0.2), depth(1.0), hasTexture(false), texture(nullptr) {
    material.ambient[0] = 0.2f; material.ambient[1] = 0.2f; material.ambient[2] = 0.2f; material.ambient[3] = 1.0f;
    material.diffuse[0] = 0.5f; material.diffuse[1] = 0.5f; material.diffuse[2] = 0.5f; material.diffuse[3] = 1.0f;
    material.specular[0] = 0.4f; material.specular[1] = 0.4f; material.specular[2] = 0.4f; material.specular[3] = 1.0f;
    material.shininess = 32.0f;
}

void Platform::Draw() {
    glPushMatrix();
    glTranslated(x, y, z);
    glScaled(width, height, depth);

    glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, material.shininess);

    if (hasTexture && texture) {
        texture->Bind();
        glBegin(GL_QUADS);
        glNormal3d(0, 1, 0);
        glTexCoord2d(0, 0); glVertex3d(-0.5, 0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(0.5, 0.5, -0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, 0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(-0.5, 0.5, 0.5);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBegin(GL_QUADS);
    // Низ
    glNormal3d(0, -1, 0);
    glVertex3d(-0.5, -0.5, -0.5);
    glVertex3d(0.5, -0.5, -0.5);
    glVertex3d(0.5, -0.5, 0.5);
    glVertex3d(-0.5, -0.5, 0.5);
    // Перед
    glNormal3d(0, 0, 1);
    glVertex3d(-0.5, -0.5, 0.5);
    glVertex3d(0.5, -0.5, 0.5);
    glVertex3d(0.5, 0.5, 0.5);
    glVertex3d(-0.5, 0.5, 0.5);
    // Зад
    glNormal3d(0, 0, -1);
    glVertex3d(-0.5, -0.5, -0.5);
    glVertex3d(-0.5, 0.5, -0.5);
    glVertex3d(0.5, 0.5, -0.5);
    glVertex3d(0.5, -0.5, -0.5);
    // Лево
    glNormal3d(-1, 0, 0);
    glVertex3d(-0.5, -0.5, -0.5);
    glVertex3d(-0.5, -0.5, 0.5);
    glVertex3d(-0.5, 0.5, 0.5);
    glVertex3d(-0.5, 0.5, -0.5);
    // Право
    glNormal3d(1, 0, 0);
    glVertex3d(0.5, -0.5, -0.5);
    glVertex3d(0.5, 0.5, -0.5);
    glVertex3d(0.5, 0.5, 0.5);
    glVertex3d(0.5, -0.5, 0.5);
    glEnd();

    glPopMatrix();
}