
#pragma once
#include <SDL2/SDL.h>

class Enemy {
public:
    Enemy(int x, int y, int w, int h, float speed, SDL_Texture* texture);
    virtual ~Enemy() = default;

    virtual void update(float deltaTime);   // virtual so subclasses override
    void render(SDL_Renderer* renderer);
    bool isOffScreen() const;
    SDL_Rect getRect() const;
    void explode();
    bool isDead() const;

protected:
    float x;
    float speed;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active = true;
    bool exploding = false;
    float explosionTimer = 0.0f;
};
