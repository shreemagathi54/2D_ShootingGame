#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED
#include <SDL2/SDL.h>

class Bullet {
private:
    float x;
    SDL_Rect rect;
    float speed;
    bool active;

public:
    Bullet();
    Bullet(int x, int y);

    void update(float deltaTime);   // fixed: uses deltaTime
    void render(SDL_Renderer* renderer);

    bool isActive() const;
    void deactivate();
    SDL_Rect getRect() const;
};

#endif // BULLET_H_INCLUDED
