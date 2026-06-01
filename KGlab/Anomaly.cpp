#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Anomaly.h"

Anomaly::Anomaly() : x(0), y(0), z(0), radius(0.35), visible(true) {
    color[0] = 0.2f;  // R
    color[1] = 0.8f;  // G
    color[2] = 1.0f;  // B
    color[3] = 0.6f;  // A (полупрозрачная)
}

void Anomaly::Draw() {
    if (!visible) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4fv(color);

    glPushMatrix();
    glTranslated(x, y, z);

    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, radius, 20, 20);
    gluDeleteQuadric(quad);

    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}