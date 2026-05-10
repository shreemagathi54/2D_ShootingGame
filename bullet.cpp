#include "bullet.h"

Bullet::Bullet() {
    x = 0;
    rect = {0, 0, 15, 8};
    speed = 600.0f;
    active = false;
}

Bullet::Bullet(int startX, int startY) {
    x = (float)startX;
    rect = {startX, startY, 15, 8};  // bigger, more visible
    speed = 600.0f;                   // pixels per second
    active = true;
}

void Bullet::update(float deltaTime) {
    x += speed * deltaTime;           // smooth movement
    rect.x = (int)x;
    if (rect.x > 800)
        active = false;
}

void Bullet::render(SDL_Renderer* renderer) {
    if (!active) return;
    // Yellow bullet with bright core
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

bool Bullet::isActive() const {
    return active;
}

void Bullet::deactivate() {
    active = false;
}

SDL_Rect Bullet::getRect() const {
    return rect;
}
