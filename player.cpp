/*#include <iostream>
#include "Player.h"
#include "bullet.h"

Player::Player(int x, int y, int w, int h)
{
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    texture = NULL;
    speed = 1;
    bulletActive=false;
}

void Player::handleInput(const Uint8* keystate)
{

    if (keystate[SDL_SCANCODE_UP])
        rect.y -= speed;

    if (keystate[SDL_SCANCODE_DOWN])
        rect.y += speed;
     if (keystate[SDL_SCANCODE_SPACE]&&!bulletActive)
    {
        bullet=Bullet(rect.x+rect.w,rect.y+rect.h/2);
        bulletActive=true;
    }

    // Keep player inside window (assuming height = 500)
    if (rect.y < 0)
        rect.y = 0;

    if (rect.y + rect.h > 700)
        rect.y = 700 - rect.h;
}

void Player::update()
{
    if (bulletActive)
    {
        bullet.update();

        if(!bullet.isActive())
            bulletActive=false;
    }
}

void Player::render(SDL_Renderer* renderer)
{
    if (!texture)
    {
        SDL_Surface* surface = SDL_LoadBMP("shooter.bmp");
        /*
        rect.w = surface->w;
        rect.h = surface->h;
*/
/*        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    if (bulletActive)
        bullet.render(renderer);
}

SDL_Rect Player::getRect() const
{
    return rect;
}
*/
#include "player.h"
#include <algorithm>

Player::Player(int x, int y, int w, int h) {
    rect.x = x; rect.y = y;
    rect.w = w; rect.h = h;
    this->y = (float)y;
    texture = NULL;
    speed = 400.0f;
    fireTimer = 0.0f;
    fireRate = 0.2f;  // fire every 0.2 seconds = 5 bullets per second
                      // change this value: lower = faster, higher = slower
}

void Player::handleInput(const Uint8* keystate, float deltaTime) {
    if (keystate[SDL_SCANCODE_UP])   y -= speed * deltaTime;
    if (keystate[SDL_SCANCODE_DOWN]) y += speed * deltaTime;

    if (y < 0) y = 0;
    if (y + rect.h > 600) y = 600 - rect.h;
    rect.y = (int)y;

    // Fire while holding space, respecting fire rate
    fireTimer += deltaTime;
    if (keystate[SDL_SCANCODE_SPACE] && fireTimer >= fireRate) {
        bullets.emplace_back(rect.x + rect.w, rect.y + rect.h / 2 - 4);
        fireTimer = 0.0f;
    }
}

void Player::update(float deltaTime) {
    for (auto &b : bullets)
        b.update(deltaTime);

    // Remove inactive bullets
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet &b) { return !b.isActive(); }),
        bullets.end()
    );
}

void Player::render(SDL_Renderer* renderer) {
    if (!texture) {
        SDL_Surface* surface = SDL_LoadBMP("shooter.bmp");
        if (surface) {
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
    if (texture)
        SDL_RenderCopy(renderer, texture, NULL, &rect);

    for (auto &b : bullets)
        b.render(renderer);
}

SDL_Rect Player::getRect() const { return rect; }

std::vector<Bullet>& Player::getBullets() { return bullets; }
