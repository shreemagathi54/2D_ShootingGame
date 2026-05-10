#pragma once
#include "Enemy.h"
#include <cmath>

class SlowEnemy : public Enemy {
public:
    SlowEnemy(int x, int y, SDL_Texture* texture)
        : Enemy(x, y, 70, 70, 60.0f, texture), baseY(y) {}

    void update(float deltaTime) override {
        wobbleTime += deltaTime;
        x -= speed * deltaTime;
        rect.x = (int)x;
        rect.y = baseY + (int)(10.0f * sinf(wobbleTime * 2.0f));
    }

private:
    float wobbleTime = 0.0f;
    int baseY;
};
