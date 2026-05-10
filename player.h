#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL2/SDL.h>
#include <vector>
#include "bullet.h"

class Player {
private:
    SDL_Rect rect;
    float y;
    float speed;
    SDL_Texture* texture;

    std::vector<Bullet> bullets;  // multiple bullets
    float fireTimer;              // tracks time between shots
    float fireRate;               // seconds between shots (lower = faster)

public:
    Player(int x, int y, int w, int h);

    void handleInput(const Uint8* keystate, float deltaTime);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    SDL_Rect getRect() const;
    std::vector<Bullet>& getBullets();  // for collision detection
};

#endif // PLAYER_H_INCLUDED
