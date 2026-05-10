#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Enemy.h"

class EnemyManager {
public:
    EnemyManager(SDL_Renderer* renderer, int width, int height, std::vector<SDL_Texture*> textures);

    void update(float deltaTime);
    void render();
    bool checkCollision(SDL_Rect bulletRect);

    bool isGameOver = false;
    int points = 0;

private:
    SDL_Renderer* renderer;
    std::vector<SDL_Texture*> textures;
    std::vector<std::unique_ptr<Enemy>> enemies;  // polymorphic container

    int windowWidth;
    int windowHeight;
    float spawnTimer;
    float spawnInterval;
    bool spawnSlow = true;  // alternates each wave
};
