#pragma once
#include "Enemy.h"
#include <cmath>

class FastEnemy : public Enemy {
public:
    FastEnemy(int x, int y, SDL_Texture* texture): Enemy(x, y, 70, 70, 220.0f, texture), baseY(y) {}

    void update(float deltaTime) override {
        zigzagTime += deltaTime;
        x -= speed * deltaTime;
        rect.x = (int)x;
        rect.y = baseY + (int)(20.0f * sinf(zigzagTime * 5.0f));
    }

private:
    float zigzagTime = 0.0f;
    int baseY;
};
