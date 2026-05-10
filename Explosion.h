#ifndef EXPLOSION_H_INCLUDED
#define EXPLOSION_H_INCLUDED
#include <SDL2/SDL.h>
#include <vector>

struct Particle {
    float x, y;
    float vx, vy;
    float life;       // 1.0 to 0.0
    float size;
    SDL_Color color;
};

class Explosion {
private:
    std::vector<Particle> particles;
    float timer;
    float duration;

public:
    Explosion(float x, float y);

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    bool isDone() const;
};

#endif // EXPLOSION_H_INCLUDED
