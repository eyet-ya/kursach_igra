#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "Render.h"
#include "MyOGL.h"
#include "GUItextRectangle.h"
#include "Texture.h"
#include "Player.h"
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include "SoundManager.h"
#include "Timer.h"

extern OpenGL gl;

Shader mainShader;
bool useShaders = true;

std::vector<SimplePlatform> platforms;

struct {
    double x, y, z;
    double radius;
    float color[4];
    bool visible;
} anomaly;

Texture gridTexture;

bool stoneMaterial = true;
bool anomalyVisible = true;
bool texturing = true;
bool lightning = true;
bool alpha = false;

Player player;

bool keys[256] = { false };

int collectedAnomalies = 0;
bool gameWin = false;

GameState gameState = STATE_MENU;
bool demoMode = true;
double demoAngle = 0;

GameTimer gameTimer;
bool victorySoundPlayed = false;

struct BezierCurve {
    std::vector<std::tuple<double, double, double>> points;
    double t;
    double speed;
    bool looped;
    bool visible;

    BezierCurve() : t(0), speed(0.3), looped(true) {}

    std::tuple<double, double, double> GetPoint(double tt) const {
        if (points.size() != 4) return { 0,0,0 };
        double x = pow(1 - tt, 3) * std::get<0>(points[0]) + 3 * pow(1 - tt, 2) * tt * std::get<0>(points[1]) + 3 * (1 - tt) * pow(tt, 2) * std::get<0>(points[2]) + pow(tt, 3) * std::get<0>(points[3]);
        double y = pow(1 - tt, 3) * std::get<1>(points[0]) + 3 * pow(1 - tt, 2) * tt * std::get<1>(points[1]) + 3 * (1 - tt) * pow(tt, 2) * std::get<1>(points[2]) + pow(tt, 3) * std::get<1>(points[3]);
        double z = pow(1 - tt, 3) * std::get<2>(points[0]) + 3 * pow(1 - tt, 2) * tt * std::get<2>(points[1]) + 3 * (1 - tt) * pow(tt, 2) * std::get<2>(points[2]) + pow(tt, 3) * std::get<2>(points[3]);
        return { x, y, z };
    }

    void Update(double dt) {
        t += speed * dt;
        if (t >= 1) {
            if (looped) t = 0;
            else { t = 1; speed = -speed; }
        }
        if (t < 0 && looped) t = 1;
    }
};

std::vector<BezierCurve> anomalies;
bool showTrajectories = false;

GuiTextRectangle text;

float stoneAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float stoneDiff[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float stoneSpec[] = { 0.4f, 0.4f, 0.4f, 1.0f };
float stoneShine = 32.0f;

float woodAmb[] = { 0.2f, 0.1f, 0.05f, 1.0f };
float woodDiff[] = { 0.6f, 0.3f, 0.1f, 1.0f };
float woodSpec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float woodShine = 16.0f;

void DrawPlatform(double x, double y, double z, double sx, double sy, double sz, bool hasTex) {
    glPushMatrix();
    glTranslated(x, y, z);  
    glScaled(sx, sy, sz);

    if (useShaders) {
        mainShader.UseShader();

        GLint locAmbient = glGetUniformLocationARB(mainShader.program, "materialAmbient");
        GLint locDiffuse = glGetUniformLocationARB(mainShader.program, "materialDiffuse");
        GLint locSpecular = glGetUniformLocationARB(mainShader.program, "materialSpecular");
        GLint locShininess = glGetUniformLocationARB(mainShader.program, "materialShininess");

        if (stoneMaterial) {
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialAmbient"), 0.2f, 0.2f, 0.2f);
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialDiffuse"), 0.5f, 0.5f, 0.5f);
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialSpecular"), 0.4f, 0.4f, 0.4f);
            glUniform1fARB(glGetUniformLocationARB(mainShader.program, "materialShininess"), 32.0f);
        }
        else {
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialAmbient"), 0.2f, 0.1f, 0.05f);
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialDiffuse"), 0.6f, 0.3f, 0.1f);
            glUniform3fARB(glGetUniformLocationARB(mainShader.program, "materialSpecular"), 0.2f, 0.2f, 0.2f);
            glUniform1fARB(glGetUniformLocationARB(mainShader.program, "materialShininess"), 16.0f);
        }
    }
    else {
        if (stoneMaterial) {
            glMaterialfv(GL_FRONT, GL_AMBIENT, stoneAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, stoneDiff);
            glMaterialfv(GL_FRONT, GL_SPECULAR, stoneSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, stoneShine);
        }
        else {
            glMaterialfv(GL_FRONT, GL_AMBIENT, woodAmb);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, woodDiff);
            glMaterialfv(GL_FRONT, GL_SPECULAR, woodSpec);
            glMaterialf(GL_FRONT, GL_SHININESS, woodShine);
        }
    }

    if (hasTex && texturing) {
        gridTexture.Bind();

        glBegin(GL_QUADS);

        // ВЕРХНЯЯ ГРАНЬ (Z+)
        glNormal3d(0, 0, 1);
        glTexCoord2d(0, 0); glVertex3d(-0.5, -0.5, 0.5);
        glTexCoord2d(1, 0); glVertex3d(0.5, -0.5, 0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, 0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(-0.5, 0.5, 0.5);

        // НИЖНЯЯ ГРАНЬ (Z-)
        glNormal3d(0, 0, -1);
        glTexCoord2d(0, 0); glVertex3d(-0.5, -0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(0.5, -0.5, -0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, 0.5, -0.5);
        glTexCoord2d(0, 1); glVertex3d(-0.5, 0.5, -0.5);

        // ПЕРЕДНЯЯ ГРАНЬ (Y+)
        glNormal3d(0, 1, 0);
        glTexCoord2d(0, 0); glVertex3d(-0.5, 0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(0.5, 0.5, -0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, 0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(-0.5, 0.5, 0.5);

        // ЗАДНЯЯ ГРАНЬ (Y-)
        glNormal3d(0, -1, 0);
        glTexCoord2d(0, 0); glVertex3d(-0.5, -0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(-0.5, -0.5, 0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, -0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(0.5, -0.5, -0.5);

        // ЛЕВАЯ ГРАНЬ (X-)
        glNormal3d(-1, 0, 0);
        glTexCoord2d(0, 0); glVertex3d(-0.5, -0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(-0.5, -0.5, 0.5);
        glTexCoord2d(1, 1); glVertex3d(-0.5, 0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(-0.5, 0.5, -0.5);

        // ПРАВАЯ ГРАНЬ (X+)
        glNormal3d(1, 0, 0);
        glTexCoord2d(0, 0); glVertex3d(0.5, -0.5, -0.5);
        glTexCoord2d(1, 0); glVertex3d(0.5, 0.5, -0.5);
        glTexCoord2d(1, 1); glVertex3d(0.5, 0.5, 0.5);
        glTexCoord2d(0, 1); glVertex3d(0.5, -0.5, 0.5);

        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
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
    }

    glPopMatrix();
}

void DrawAnomaly() {
    if (!anomaly.visible) return;

    Shader::DontUseShaders();

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4fv(anomaly.color);
    glPushMatrix();
    glTranslated(anomaly.x, anomaly.y, anomaly.z);
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, anomaly.radius, 20, 20);
    gluDeleteQuadric(quad);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void DrawBezierAnomalies() {
    Shader::DontUseShaders();
    for (size_t i = 0; i < anomalies.size(); ++i) {
        auto [ax, ay, az] = anomalies[i].GetPoint(anomalies[i].t);

        double dx = player.x - ax;
        double dy = player.y - ay;
        double dz = player.z - az;
        double dist = sqrt(dx * dx + dy * dy + dz * dz);

        if (dist < player.radius + 0.3 && anomalies[i].looped) {
            anomalies[i].looped = false;
            collectedAnomalies++;
            SoundManager::PlayCollectSound();

            glColor4f(1.0f, 0.5f, 0.0f, 1.0f);
            glBegin(GL_LINES);
            glVertex3d(ax - 0.2, ay, az);
            glVertex3d(ax + 0.2, ay, az);
            glEnd();

            continue;
        }

        if (!anomalies[i].looped) continue;

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.2f, 0.8f, 1.0f, 0.6f);
        glPushMatrix();
        glTranslated(ax, ay, az);
        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, 0.3, 20, 20);
        gluDeleteQuadric(quad);
        glPopMatrix();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        if (showTrajectories) {
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glColor3f(1, 1, 0);
            glBegin(GL_LINE_STRIP);
            for (double t = 0; t <= 1; t += 0.05) {
                auto [px, py, pz] = anomalies[i].GetPoint(t);
                glVertex3d(px, py, pz);
            }
            glEnd();
            glEnable(GL_LIGHTING);
        }
    }
}

void handleInput(double deltaTime) {
    if (gameState != STATE_GAME) return;
    if (gameWin) return;
    double speed = 5.0;
    double dx = 0, dy = 0;
    if (keys['A']) dy = 1.0;
    if (keys['D']) dy = -1.0;
    if (keys['W']) dx = -1.0;
    if (keys['S']) dx = 1.0;
    dy = -dy;
    if (dx != 0 && dy != 0) {
        double len = sqrt(dx * dx + dy * dy);
        dx /= len;
        dy /= len;
    }
    double angle = player.yaw;
    double cosA = cos(angle);
    double sinA = sin(angle);
    double worldDX = dx * cosA - dy * sinA;
    double worldDY = dx * sinA + dy * cosA;
    player.vx = worldDX * speed;
    player.vy = worldDY * speed;
    if (keys[VK_SPACE] && player.onGround) {
        player.Jump();
        SoundManager::PlayJumpSound();
        keys[VK_SPACE] = false;
    }
}

void onKeyDown(OpenGL* sender, KeyEventArg arg) {
    switch (arg.key) {
    case 'W': keys['W'] = true; break;
    case 'A': keys['A'] = true; break;
    case 'S': keys['S'] = true; break;
    case 'D': keys['D'] = true; break;
    case VK_SPACE: keys[VK_SPACE] = true; break;
    case 'R':
        collectedAnomalies = 0;
        gameWin = false;
        victorySoundPlayed = false;

        if (gameState == STATE_PAUSE || gameState == STATE_GAMEOVER_WIN || gameState == STATE_GAMEOVER_LOSE) {
            gameTimer.Reset();
            gameState = STATE_GAME;
        }
        else if (gameState == STATE_MENU) {
            gameTimer.Reset();
            gameState = STATE_GAME;
            demoMode = false;
        }
        else if (gameState == STATE_GAME) {
            gameTimer.Reset();
        }

        player.x = 0; player.y = 0; player.z = -0.1;
        player.vx = 0; player.vy = 0; player.vz = 0;
        player.onGround = true;

        for (auto& a : anomalies) {
            a.looped = true;
            a.t = 0;
        }
        break;

    case VK_RETURN:
        if (gameState == STATE_MENU) {
            gameState = STATE_GAME;
            demoMode = false;
            gameTimer.Start(60.0);
            victorySoundPlayed = false;
            collectedAnomalies = 0;
            gameWin = false;
            for (auto& a : anomalies) {
                a.looped = true;
                a.t = 0;
            }
            player.x = 0; player.y = 0; player.z = -0.1;
            player.vx = 0; player.vy = 0; player.vz = 0;
            player.onGround = true;
        }
        else if (gameState == STATE_PAUSE) {
            gameState = STATE_GAME;
            gameTimer.Resume();
        }
        else if (gameState == STATE_GAMEOVER_WIN || gameState == STATE_GAMEOVER_LOSE) {
            gameState = STATE_MENU;
            gameTimer.Stop();
            demoMode = true;
            victorySoundPlayed = false;
        }
        break;

    case VK_ESCAPE:
        if (gameState == STATE_GAME) {
            gameState = STATE_PAUSE;
            gameTimer.Pause();
        }
        else if (gameState == STATE_PAUSE) {
            gameState = STATE_GAME;
            gameTimer.Resume();
        }
        break;
    }
}

void onKeyUp(OpenGL* sender, KeyEventArg arg) {
    switch (arg.key) {
    case 'W': keys['W'] = false; break;
    case 'A': keys['A'] = false; break;
    case 'S': keys['S'] = false; break;
    case 'D': keys['D'] = false; break;
    case VK_SPACE: keys[VK_SPACE] = false; break;
    }
}

void switchModes(OpenGL* sender, KeyEventArg arg) {
    auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));
    switch (key) {
    case 'L': lightning = !lightning; break;
    case 'T': texturing = !texturing; break;
    case 'A': alpha = !alpha; break;
    case 'M': stoneMaterial = !stoneMaterial; break;
    case 'P': anomalyVisible = !anomalyVisible; anomaly.visible = anomalyVisible; break;
    case 'Y': showTrajectories = !showTrajectories; break;
    }
}

void initAnomalies() {
    anomalies.clear();

    BezierCurve c;

    //  УРОВЕНЬ 1 

    // Северная аномалия (прямоугольник)
    c.points = { {-1, 6, 1.2}, {0, 7, 1.4}, {1, 6, 1.2}, {-1, 6, 1.2} };
    c.speed = 0.25;
    anomalies.push_back(c);

    // Южная аномалия
    c.points = { {1, -6, 1.2}, {0, -7, 1.4}, {-1, -6, 1.2}, {1, -6, 1.2} };
    c.speed = 0.25;
    anomalies.push_back(c);

    // Западная аномалия
    c.points = { {-6, 1, 1.2}, {-7, 0, 1.4}, {-6, -1, 1.2}, {-6, 1, 1.2} };
    c.speed = 0.25;
    anomalies.push_back(c);

    // Восточная аномалия
    c.points = { {6, -1, 1.2}, {7, 0, 1.4}, {6, 1, 1.2}, {6, -1, 1.2} };
    c.speed = 0.25;
    anomalies.push_back(c);

    //  УРОВЕНЬ 2 

    c.points = { {3, 3, 2.7}, {4, 4, 2.9}, {3, 5, 2.7}, {3, 3, 2.7} };
    c.speed = 0.32;
    anomalies.push_back(c);

    c.points = { {-3, 3, 2.7}, {-4, 4, 2.9}, {-5, 3, 2.7}, {-3, 3, 2.7} };
    c.speed = 0.32;
    anomalies.push_back(c);

    c.points = { {3, -3, 2.7}, {4, -4, 2.9}, {3, -5, 2.7}, {3, -3, 2.7} };
    c.speed = 0.32;
    anomalies.push_back(c);

    c.points = { {-3, -3, 2.7}, {-4, -4, 2.9}, {-5, -3, 2.7}, {-3, -3, 2.7} };
    c.speed = 0.32;
    anomalies.push_back(c);

    // УРОВЕНЬ 3 

    c.points = { {0, 5, 4.2}, {1, 6, 4.4}, {0, 7, 4.2}, {0, 5, 4.2} };
    c.speed = 0.38;
    anomalies.push_back(c);

    c.points = { {0, -5, 4.2}, {-1, -6, 4.4}, {0, -7, 4.2}, {0, -5, 4.2} };
    c.speed = 0.38;
    anomalies.push_back(c);

    c.points = { {5, 0, 4.2}, {6, 1, 4.4}, {7, 0, 4.2}, {5, 0, 4.2} };
    c.speed = 0.38;
    anomalies.push_back(c);

    c.points = { {-5, 0, 4.2}, {-6, -1, 4.4}, {-7, 0, 4.2}, {-5, 0, 4.2} };
    c.speed = 0.38;
    anomalies.push_back(c);

    // УРОВЕНЬ 4 

    c.points = { {2, 2, 5.7}, {3, 3, 5.9}, {2, 4, 5.7}, {2, 2, 5.7} };
    c.speed = 0.42;
    anomalies.push_back(c);

    c.points = { {-2, 2, 5.7}, {-3, 3, 5.9}, {-4, 2, 5.7}, {-2, 2, 5.7} };
    c.speed = 0.42;
    anomalies.push_back(c);

    c.points = { {2, -2, 5.7}, {3, -3, 5.9}, {2, -4, 5.7}, {2, -2, 5.7} };
    c.speed = 0.42;
    anomalies.push_back(c);

    c.points = { {-2, -2, 5.7}, {-3, -3, 5.9}, {-4, -2, 5.7}, {-2, -2, 5.7} };
    c.speed = 0.42;
    anomalies.push_back(c);

    // УРОВЕНЬ 5 

    c.points = { {0, 0, 7.2}, {1, 1, 7.4}, {0, 2, 7.2}, {0, 0, 7.2} };
    c.speed = 0.5;
    anomalies.push_back(c);

}

void initRender() {
    gameState = STATE_MENU;
    gridTexture.LoadTexture("textures/asphalt.png");

    mainShader.VshaderFileName = "shaders/vertex.vert";
    mainShader.FshaderFileName = "shaders/fragment.frag";
    mainShader.LoadShaderFromFile();
    mainShader.Compile();

    mainShader.UseShader();
    GLint loc = glGetUniformLocationARB(mainShader.program, "texture0");
    glUniform1iARB(loc, 0);  

    float lightAmbient[3] = { 0.2f, 0.2f, 0.2f };
    float lightDiffuse[3] = { 0.8f, 0.8f, 0.8f };
    float lightSpecular[3] = { 1.0f, 1.0f, 1.0f };

    glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightAmbient"), 1, lightAmbient);
    glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightDiffuse"), 1, lightDiffuse);
    glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightSpecular"), 1, lightSpecular);

    gridTexture.LoadTexture("textures/asphalt.png");

    platforms.clear();

    // арена
    SimplePlatform p;

    // большая платформа в центре
    p.x = 0; p.y = 0; p.z = -0.5;
    p.sizeX = 18.0; p.sizeY = 18.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);


    // СТОЛБИКИ
    
    double arenaSize = 9.0;      
    double pillarSpacing = 0.5;  
    double pillarHeight = 1.5;   
    double pillarWidth = 0.4;    
    
    // СТОЛБИКИ ПО ВЕРХНЕМУ КРАЮ 
    for (double x = -arenaSize + pillarSpacing; x <= arenaSize - pillarSpacing; x += pillarSpacing) {
        p.x = x; p.y = arenaSize; p.z = pillarHeight / 4;
        p.sizeX = pillarWidth; p.sizeY = pillarWidth; p.sizeZ = pillarHeight;
        p.hasTexture = true;
        platforms.push_back(p);
    }
    
    // СТОЛБИКИ ПО НИЖНЕМУ КРАЮ 
    for (double x = -arenaSize + pillarSpacing; x <= arenaSize - pillarSpacing; x += pillarSpacing) {
        p.x = x; p.y = -arenaSize; p.z = pillarHeight / 4;
        p.sizeX = pillarWidth; p.sizeY = pillarWidth; p.sizeZ = pillarHeight;
        p.hasTexture = true;
        platforms.push_back(p);
    }
    
    // СТОЛБИКИ ПО ЛЕВОМУ КРАЮ 
    for (double y = -arenaSize + pillarSpacing; y <= arenaSize - pillarSpacing; y += pillarSpacing) {
        p.x = -arenaSize; p.y = y; p.z = pillarHeight / 4;
        p.sizeX = pillarWidth; p.sizeY = pillarWidth; p.sizeZ = pillarHeight;
        p.hasTexture = true;
        platforms.push_back(p);
    }
    
    // СТОЛБИКИ ПО ПРАВОМУ КРАЮ 
    for (double y = -arenaSize + pillarSpacing; y <= arenaSize - pillarSpacing; y += pillarSpacing) {
        p.x = arenaSize; p.y = y; p.z = pillarHeight / 4;
        p.sizeX = pillarWidth; p.sizeY = pillarWidth; p.sizeZ = pillarHeight;
        p.hasTexture = true;
        platforms.push_back(p);
    }
    
    // УГЛОВЫЕ СТОЛБИКИ (побольше) 
    double cornerSize = 0.6;
    double cornerHeight = 1.8;
    
    p.x = arenaSize; p.y = arenaSize; p.z = cornerHeight / 4;
    p.sizeX = cornerSize; p.sizeY = cornerSize; p.sizeZ = cornerHeight;
    p.hasTexture = true;
    platforms.push_back(p);
    
    p.x = arenaSize; p.y = -arenaSize; p.z = cornerHeight / 4;
    p.sizeX = cornerSize; p.sizeY = cornerSize; p.sizeZ = cornerHeight;
    p.hasTexture = true;
    platforms.push_back(p);
    
    p.x = -arenaSize; p.y = arenaSize; p.z = cornerHeight / 4;
    p.sizeX = cornerSize; p.sizeY = cornerSize; p.sizeZ = cornerHeight;
    p.hasTexture = true;
    platforms.push_back(p);
    
    p.x = -arenaSize; p.y = -arenaSize; p.z = cornerHeight / 4;
    p.sizeX = cornerSize; p.sizeY = cornerSize; p.sizeZ = cornerHeight;
    p.hasTexture = true;
    platforms.push_back(p);

    // УРОВЕНЬ 1 платформы по краям 

    p.x = 0; p.y = 6; p.z = 0.8;
    p.sizeX = 2.5; p.sizeY = 2.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 0; p.y = -6; p.z = 0.8;
    p.sizeX = 2.5; p.sizeY = 2.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 6; p.y = 0; p.z = 0.8;
    p.sizeX = 2.5; p.sizeY = 2.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -6; p.y = 0; p.z = 0.8;
    p.sizeX = 2.5; p.sizeY = 2.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // УРОВЕНЬ 2 

    p.x = 0; p.y = 4; p.z = 2.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 0; p.y = -4; p.z = 2.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 4; p.y = 0; p.z = 2.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -4; p.y = 0; p.z = 2.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 0; p.y = 0; p.z = 2.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    //  УРОВЕНЬ 3 

    p.x = 3; p.y = 3; p.z = 3.8;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 3; p.y = -3; p.z = 3.8;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -3; p.y = 3; p.z = 3.8;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -3; p.y = -3; p.z = 3.8;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 5; p.y = 5; p.z = 3.8;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // УРОВЕНЬ 4 

    p.x = 0; p.y = 5; p.z = 5.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 0; p.y = -5; p.z = 5.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 5; p.y = 0; p.z = 5.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -5; p.y = 0; p.z = 5.3;
    p.sizeX = 2.0; p.sizeY = 2.0; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    //  УРОВЕНЬ 5 

    p.x = 0; p.y = 0; p.z = 6.8;
    p.sizeX = 2.5; p.sizeY = 2.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // ПЛАТФОРМЫ-ЛИФТЫ

    // Между уровнем 1 и 2
    p.x = 2; p.y = 5; p.z = 1.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = 5; p.z = 1.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 2; p.y = -5; p.z = 1.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = -5; p.z = 1.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // Между уровнем 2 и 3
    p.x = 4; p.y = 2; p.z = 3.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -4; p.y = 2; p.z = 3.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 4; p.y = -2; p.z = 3.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -4; p.y = -2; p.z = 3.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // Между уровнем 3 и 4
    p.x = 2; p.y = 5; p.z = 4.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = 5; p.z = 4.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 2; p.y = -5; p.z = 4.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = -5; p.z = 4.6;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    // Между уровнем 4 и 5
    p.x = 2; p.y = 2; p.z = 6.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = 2; p.z = 6.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = 2; p.y = -2; p.z = 6.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    p.x = -2; p.y = -2; p.z = 6.0;
    p.sizeX = 1.5; p.sizeY = 1.5; p.sizeZ = 0.2;
    p.hasTexture = true;
    platforms.push_back(p);

    anomaly.x = -2.0;
    anomaly.y = 2.0;
    anomaly.z = 1.6;
    anomaly.radius = 0.35;
    anomaly.color[0] = 0.2f;
    anomaly.color[1] = 0.8f;
    anomaly.color[2] = 1.0f;
    anomaly.color[3] = 0.6f;
    anomaly.visible = true;

    light.SetPosition(8.0, 8.0, 15.0);
    camera.setPosition(10, 10, 8);

    player.x = 0;
    player.y = 0;
    player.z = -0.1;
    player.LoadModel();

    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);
    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);
    gl.KeyDownEvent.reaction(switchModes);
    gl.KeyDownEvent.reaction(onKeyDown);
    gl.KeyUpEvent.reaction(onKeyUp);

    initAnomalies();

    text.setSize(512, 320);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

double full_time = 0;

void Render(double delta_time) {
    full_time += delta_time;

    if (gameState == STATE_GAME) {
        gameTimer.Update();
    }

    handleInput(delta_time);

    if (gameState == STATE_GAME) {
        player.Update(delta_time, platforms);
        for (auto& a : anomalies) a.Update(delta_time);
    }
    else if (gameState == STATE_PAUSE) {
    }
    else {
        for (auto& a : anomalies) a.Update(delta_time);
    }

    if (gameState == STATE_GAME) {
        if (collectedAnomalies >= 17 && !gameWin) {
            gameWin = true;
            gameTimer.Stop();
            gameState = STATE_GAMEOVER_WIN;
            if (!victorySoundPlayed) {
                SoundManager::PlayVictorySound();
                victorySoundPlayed = true;
            }
        }

        if (gameTimer.IsTimeUp() && collectedAnomalies < 17 && !gameWin) {
            gameState = STATE_GAMEOVER_LOSE;
            gameTimer.Stop();
            if (!victorySoundPlayed) {  
                SoundManager::PlayLoseSound();
                victorySoundPlayed = true;  
            }
        }
    }

    if (gameState == STATE_MENU && demoMode) {
        demoAngle += delta_time * 0.5;
        double radius = 8.0;
        double camX = sin(demoAngle) * radius;
        double camY = cos(demoAngle * 0.7) * radius * 0.6;
        double camZ = 3.0 + cos(demoAngle) * radius * 0.5;
        camera.setPosition(camX, camY, camZ);
        camera.SetUpCamera();
    }
    else if (gameState == STATE_GAME || gameState == STATE_PAUSE || gameState == STATE_GAMEOVER_WIN || gameState == STATE_GAMEOVER_LOSE) {
        camera.FollowPlayer(player.x, player.y, player.z);
        player.SetYaw(camera.fi1());
        camera.SetUpCamera();
    }

    if (useShaders) {
        mainShader.UseShader();
        float lightPos[3] = { (float)light.x(), (float)light.y(), (float)light.z() };
        glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightPos"), 1, lightPos);

        float lightAmbient[3] = { 0.2f, 0.2f, 0.2f };
        float lightDiffuse[3] = { 0.8f, 0.8f, 0.8f };
        float lightSpecular[3] = { 1.0f, 1.0f, 1.0f };

        glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightAmbient"), 1, lightAmbient);
        glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightDiffuse"), 1, lightDiffuse);
        glUniform3fvARB(glGetUniformLocationARB(mainShader.program, "lightSpecular"), 1, lightSpecular);
    }
    else {
        Shader::DontUseShaders();
    }

    light.SetUpLight();
    gl.DrawAxes();

    glEnable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    if (lightning && !useShaders) glEnable(GL_LIGHTING);
    if (texturing) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, 0); }
    if (alpha) { glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }

    if (!useShaders) {
        float defaultAmb[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        float defaultDiff[] = { 0.6f, 0.6f, 0.6f, 1.0f };
        float defaultSpec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT, defaultAmb);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, defaultDiff);
        glMaterialfv(GL_FRONT, GL_SPECULAR, defaultSpec);
        glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);
        glShadeModel(GL_SMOOTH);
    }

    for (size_t i = 0; i < platforms.size(); ++i) {
        DrawPlatform(platforms[i].x, platforms[i].y, platforms[i].z,
            platforms[i].sizeX, platforms[i].sizeY, platforms[i].sizeZ,
            platforms[i].hasTexture);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    DrawBezierAnomalies();
    player.Draw();

    glBindTexture(GL_TEXTURE_2D, 0);
    glLoadIdentity();
    camera.SetUpCamera();
    light.DrawLightGizmo();

    Shader::DontUseShaders();
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::wstringstream ss;
    ss << L"=== УПРАВЛЕНИЕ ===\n"
        << L"WASD - движение\n"
        << L"Space - прыжок\n"
        << L"Y - траектории: " << (showTrajectories ? L"ВКЛ" : L"ВЫКЛ") << L"\n"
        << L"G+мышь - двигать свет\n"
        << L"R - перезапуск игры\n\n"
        << L"Счёт: " << collectedAnomalies << L" / 17\n\n";

    if (gameState == STATE_GAME || gameState == STATE_PAUSE) {
        ss << L"══════════════════════\n";
        ss << L"    ОСТАЛОСЬ ВРЕМЕНИ\n";
        ss << L"       " << gameTimer.GetFormattedTime() << L"\n";
        ss << L"══════════════════════\n\n";
    }

    ss << L"Свет: (" << std::setw(6) << light.x() << L"," << std::setw(6) << light.y() << L"," << std::setw(6) << light.z() << L")\n"
        << L"Камера: (" << std::setw(6) << camera.x() << L"," << std::setw(6) << camera.y() << L"," << std::setw(6) << camera.z() << L")\n"
        << L"Игрок: (" << std::setw(6) << player.x << L"," << std::setw(6) << player.y << L"," << std::setw(6) << player.z << L")\n"
        << L"Delta: " << std::setprecision(5) << delta_time << L" sec";

    text.setPosition(10, gl.getHeight() - 10 - 320);
    text.setText(ss.str().c_str(), 0, 255, 0);
    text.Draw();

    int centerX = gl.getWidth() / 2;
    int centerY = gl.getHeight() / 2;

    if (gameState == STATE_MENU) {
        static GuiTextRectangle menuText;
        static bool menuInit = false;
        if (!menuInit) {
            menuText.setSize(400, 100);
            menuInit = true;
        }
        menuText.setPosition(centerX - 200, centerY - 50);
        menuText.setText(L"НАЖМИТЕ ENTER ДЛЯ СТАРТА",0, 255, 0);
        menuText.Draw();
    }
    else if (gameState == STATE_PAUSE) {
        static GuiTextRectangle pauseText;
        static bool pauseInit = false;
        if (!pauseInit) {
            pauseText.setSize(400, 100);
            pauseInit = true;
        }
        pauseText.setPosition(centerX - 200, centerY - 50);
        pauseText.setText(L"ПАУЗА. НАЖМИТЕ ENTER", 0, 255, 0);
        pauseText.Draw();
    }
    else if (gameState == STATE_GAMEOVER_WIN) {
        static GuiTextRectangle winText;
        static bool winInit = false;
        if (!winInit) {
            winText.setSize(500, 100);
            winInit = true;
        }
        winText.setPosition(centerX - 250, centerY - 50);
        winText.setText(L"★★★★★ ПОБЕДА! ★★★★★\n\nНАЖМИТЕ R ДЛЯ МЕНЮ", 0, 255, 0);
        winText.Draw();
    }
    else if (gameState == STATE_GAMEOVER_LOSE) {
        static GuiTextRectangle loseText;
        static bool loseInit = false;
        if (!loseInit) {
            loseText.setSize(500, 100);
            loseInit = true;
        }
        loseText.setPosition(centerX - 250, centerY - 50);
        loseText.setText(L"ВРЕМЯ ВЫШЛО!\n\nНАЖМИТЕ R ДЛЯ МЕНЮ", 0, 255, 0);
        loseText.Draw();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
