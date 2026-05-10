#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

#include <vector>
#include <SDL2/SDL.h>

struct Star {
    float x, y;
    float speed;
    int size;
    int brightness;
};

class Background {
private:
    std::vector<Star> stars;
    int windowWidth;
    int windowHeight;

public:
    Background(int count, int width, int height);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
};

#endif
