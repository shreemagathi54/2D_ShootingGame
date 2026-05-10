#include "Background.h"
#include <cstdlib>

Background::Background(int count, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    stars.resize(count);
    for (auto &s : stars) {
        s.x = (float)(rand() % windowWidth);
        s.y = (float)(rand() % windowHeight);
        s.speed = 30.0f + rand() % 100;   // different speeds = depth effect
        s.size = (s.speed > 80) ? 2 : 1;  // faster = bigger (closer)
        s.brightness = 100 + rand() % 156;
    }
}

void Background::update(float deltaTime) {
    for (auto &s : stars) {
        s.x -= s.speed * deltaTime;
        if (s.x < 0) {
            s.x = (float)windowWidth; // wrap to right side
            s.y = (float)(rand() % windowHeight);
        }
    }
}

void Background::render(SDL_Renderer* renderer) {
    for (auto &s : stars) {
        int b = s.brightness;
        SDL_SetRenderDrawColor(renderer, b, b, b, 55);
        SDL_Rect sr = {(int)s.x, (int)s.y, s.size, s.size};
        SDL_RenderFillRect(renderer, &sr);
    }
}
