#pragma once
#include "MyShaders.h"
#include "Timer.h"  

enum GameState {
    STATE_MENU,
    STATE_GAME,
    STATE_PAUSE,
    STATE_GAMEOVER_WIN,    
    STATE_GAMEOVER_LOSE    
};

struct SimplePlatform {
    double x, y, z;
    double sizeX, sizeY, sizeZ;
    bool hasTexture;
};

extern Shader mainShader;
extern bool useShaders;
extern GameState gameState;

void initRender();
void Render(double);

extern GameTimer gameTimer;