#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <direct.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include "EnemyManager.h"
#include "player.h"
#include "Explosion.h"

#undef main

struct Star {
    float x, y;
    float speed;
    int size;
    int brightness;
};

int main() {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL_Init failed: " << SDL_GetError() << "\n";
        return -1;
    }
    if (TTF_Init() == -1) {
        std::cout << "TTF Init Error: " << TTF_GetError() << "\n";
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer Error: " << Mix_GetError() << "\n";
        return -1;
    }

    Mix_Chunk* shootSound     = Mix_LoadWAV("shoot.wav");
    Mix_Chunk* explosionSound = Mix_LoadWAV("explosion.wav");
    Mix_Chunk* gameOverSound  = Mix_LoadWAV("gameover.wav");

    char cwd[512];
    _getcwd(cwd, sizeof(cwd));
    std::cout << "Working directory: " << cwd << "\n";

    SDL_Window* window = SDL_CreateWindow("Alien Shooter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!window) { SDL_Quit(); return -1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { SDL_DestroyWindow(window); SDL_Quit(); return -1; }

    TTF_Font* font1 = TTF_OpenFont("arialceb.ttf", 38);
    TTF_Font* font2 = TTF_OpenFont("arialceb.ttf", 18);

    // Stars
    srand(42);
    const int STAR_COUNT = 250;
    std::vector<Star> stars(STAR_COUNT);
    for (auto &s : stars) {
        s.x = (float)(rand() % 800);
        s.y = (float)(rand() % 600);
        s.speed = 30.0f + rand() % 100;
        s.size = (s.speed > 80) ? 2 : 1;
        s.brightness = 100 + rand() % 156;
    }

    // Textures
    std::vector<SDL_Texture*> textures;
    std::vector<std::string> paths = {
        "red.bmp", "green.bmp", "orange.bmp", "blue.bmp", "purple.bmp"
    };
    for (auto &path : paths) {
        SDL_Surface* surface = SDL_LoadBMP(path.c_str());
        if (!surface) { std::cout << "Failed: " << path << "\n"; continue; }
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (tex) textures.push_back(tex);
    }
    if (textures.empty()) { std::cout << "No textures loaded.\n"; return -1; }

    Player player(10, 270, 80, 60);
    EnemyManager enemyManager(renderer, 800, 600, textures);
    std::vector<Explosion> explosions;

    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    int prevBulletCount = 0;
    bool gameOverSoundPlayed = false;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        player.handleInput(keystate, deltaTime);
        player.update(deltaTime);
        enemyManager.update(deltaTime);

        // Shoot sound
        int currentBulletCount = (int)player.getBullets().size();
        if (currentBulletCount > prevBulletCount && shootSound)
            Mix_PlayChannel(-1, shootSound, 0);
        prevBulletCount = currentBulletCount;

        // Collision + fireball explosion
        for (auto &bullet : player.getBullets()) {
            if (bullet.isActive()) {
                SDL_Rect br = bullet.getRect();
                if (enemyManager.checkCollision(br)) {
                    bullet.deactivate();

                    // Spawn fireball at enemy center
                    explosions.emplace_back(
                        (float)(br.x + br.w / 2),
                        (float)(br.y + br.h / 2)
                    );

                    if (explosionSound)
                        Mix_PlayChannel(-1, explosionSound, 0);
                }
            }
        }

        // Update and clean explosions
        for (auto &exp : explosions)
            exp.update(deltaTime);
        explosions.erase(
            std::remove_if(explosions.begin(), explosions.end(),
                [](Explosion &e) { return e.isDone(); }),
            explosions.end()
        );

        // Move stars
        for (auto &s : stars) {
            s.x -= s.speed * deltaTime;
            if (s.x < 0) { s.x = 800.0f; s.y = (float)(rand() % 600); }
        }

        // Game Over
        if (enemyManager.isGameOver) {
            if (!gameOverSoundPlayed && gameOverSound) {
                Mix_PlayChannel(-1, gameOverSound, 0);
                gameOverSoundPlayed = true;
            }

            SDL_Window* goWindow = SDL_CreateWindow("Game Over",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                600, 400, SDL_WINDOW_SHOWN);
            SDL_Renderer* goRenderer = SDL_CreateRenderer(
                goWindow, -1, SDL_RENDERER_ACCELERATED);

            std::fstream fstrm;
            fstrm.open("scores", std::ios::app);
            fstrm << std::to_string(enemyManager.points) + "\n";
            fstrm.close();

            fstrm.open("scores", std::ios::in);
            int score, highscore = enemyManager.points;
            while (fstrm >> score)
                if (score > highscore) highscore = score;
            fstrm.close();

            bool overRunning = true;
            bool restart = false;
            SDL_Event e;

            while (overRunning) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) overRunning = false;
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_r) { overRunning = false; restart = true; }
                        if (e.key.keysym.sym == SDLK_e) { overRunning = false; restart = false; }
                    }
                }

                SDL_SetRenderDrawColor(goRenderer, 0, 0, 0, 255);
                SDL_RenderClear(goRenderer);

                SDL_Color red    = {255, 0,   0,   255};
                SDL_Color white  = {255, 255, 255, 255};
                SDL_Color yellow = {255, 255, 0,   255};

                auto renderText = [&](TTF_Font* font, const char* text,
                                      SDL_Color color, int x, int y) {
                    SDL_Surface* surf = TTF_RenderText_Solid(font, text, color);
                    if (!surf) return;
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(goRenderer, surf);
                    SDL_Rect rect = {x, y, surf->w, surf->h};
                    SDL_FreeSurface(surf);
                    SDL_RenderCopy(goRenderer, tex, NULL, &rect);
                    SDL_DestroyTexture(tex);
                };

                renderText(font1, "GAME OVER", red, 180, 120);
                renderText(font2, ("Score: " + std::to_string(enemyManager.points)).c_str(), white, 150, 200);
                renderText(font2, ("High Score: " + std::to_string(highscore)).c_str(), white, 325, 200);
                renderText(font2, "Press R to Restart", yellow, 120, 300);
                renderText(font2, "Press E to Exit", yellow, 335, 300);

                SDL_RenderPresent(goRenderer);
            }

            SDL_DestroyRenderer(goRenderer);
            SDL_DestroyWindow(goWindow);

            if (restart) {
                enemyManager = EnemyManager(renderer, 800, 600, textures);
                explosions.clear();
                gameOverSoundPlayed = false;
            } else {
                running = false;
            }
        }

        // ===== RENDER =====
        SDL_SetRenderDrawColor(renderer, 0, 0, 10, 255);
        SDL_RenderClear(renderer);

        // Stars
        for (auto &s : stars) {
            int b = s.brightness;
            SDL_SetRenderDrawColor(renderer, b, b, b, 255);
            SDL_Rect sr = {(int)s.x, (int)s.y, s.size, s.size};
            SDL_RenderFillRect(renderer, &sr);
        }

        player.render(renderer);
        enemyManager.render();

        // Render fireball explosions on top
        for (auto &exp : explosions)
            exp.render(renderer);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(explosionSound);
    Mix_FreeChunk(gameOverSound);
    Mix_CloseAudio();
    TTF_CloseFont(font1);
    TTF_CloseFont(font2);
    TTF_Quit();
    for (auto tex : textures)
        SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
