#include "Enemy.h"

Enemy::Enemy(int x, int y, int w, int h, float speed, SDL_Texture* texture) {
    this->x = (float)x;
    rect = {x, y, w, h};
    this->speed = speed;
    this->texture = texture;
    this->active = true;
}

void Enemy::update(float deltaTime) {
    x -= speed * deltaTime;
    rect.x = (int)x;
}

void Enemy::render(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

bool Enemy::isOffScreen() const {
    return rect.x + rect.w < 0;
}

SDL_Rect Enemy::getRect() const {
    return rect;
}

void Enemy::explode() {
    exploding = true;
    explosionTimer = 0.0f;
}

bool Enemy::isDead() const {
    return !active;
}
