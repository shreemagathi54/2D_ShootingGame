#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <direct.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>
#include "EnemyManager.h"
#include "player.h"

#undef main

// ===== STAR =====
struct Star {
    float x, y, speed;
    int size, brightness;
};

// ===== EXPLOSION =====
// Mimics sprite sheet: small orange burst → big fireball → dark smoke ring
// ===== EXPLOSION =====
struct Explosion {
    float cx, cy;
    float timer;
    float duration;
    int   size;

    // Pre-baked debris — 16 chunks, fixed at construction
    struct Chunk {
        float angle;
        float distMult; // multiplier on radius
        int   sz;
        Uint8 r, g, b;
    };
    Chunk chunks[16];

    Explosion(float x, float y) : cx(x), cy(y), timer(0.0f), duration(0.65f), size(70) {
        // Fixed angles spread evenly + small offsets baked as constants
        float offsets[16] = {
            0.0f,  0.42f, 0.81f, 1.18f, 1.62f, 2.05f, 2.44f, 2.88f,
            3.28f, 3.67f, 4.10f, 4.52f, 4.89f, 5.24f, 5.61f, 5.98f
        };
        float distMults[16] = {
            0.90f,1.05f,0.85f,1.10f,0.95f,1.00f,0.88f,1.08f,
            0.92f,1.03f,0.87f,1.07f,0.94f,1.01f,0.89f,1.06f
        };
        int sizes[16] = { 5,7,4,8,6,5,7,4, 6,8,5,7,4,6,8,5 };
        // Alternate orange, yellow, deep-orange per chunk
        Uint8 rs[3] = {255, 255, 200};
        Uint8 gs[3] = {110, 210,  55};
        Uint8 bs[3] = {  0,  30,   0};

        for (int i = 0; i < 16; i++) {
            chunks[i].angle    = offsets[i];
            chunks[i].distMult = distMults[i];
            chunks[i].sz       = sizes[i];
            chunks[i].r        = rs[i % 3];
            chunks[i].g        = gs[i % 3];
            chunks[i].b        = bs[i % 3];
        }
    }

    void update(float dt) { timer += dt; }
    bool isDone() const   { return timer >= duration; }

    // ── helpers ──────────────────────────────────────────────────────────
    void fillCircle(SDL_Renderer* rnd, int x, int y, int r) {
        if (r <= 0) return;
        for (int dy = -r; dy <= r; dy++) {
            int dx = (int)sqrtf((float)(r*r - dy*dy));
            SDL_RenderDrawLine(rnd, x-dx, y+dy, x+dx, y+dy);
        }
    }

    // Lumpy irregular blob — 4 offset circles merged together
    void fillBlob(SDL_Renderer* rnd, int x, int y, int r) {
        if (r <= 0) return;
        fillCircle(rnd, x,        y,        r);
        fillCircle(rnd, x+r/4,    y-r/5,    r*3/4);
        fillCircle(rnd, x-r/3,    y+r/4,    r*2/3);
        fillCircle(rnd, x+r/5,    y+r/3,    r*2/3);
    }

    // ── render ────────────────────────────────────────────────────────────
    void render(SDL_Renderer* rnd) {
        if (isDone()) return;

        float t = timer / duration;   // 0 → 1
        int   R = size / 2;           // 35px base radius

        SDL_SetRenderDrawBlendMode(rnd, SDL_BLENDMODE_BLEND);

        // ── PHASE 1  (t: 0.00 → 0.25)  initial spark burst ──────────────
        if (t < 0.25f) {
            float p = t / 0.25f;                       // 0→1 within phase
            int   r = (int)(R * 0.55f * p);

            // white-hot core
            SDL_SetRenderDrawColor(rnd, 255,255,220,255);
            fillCircle(rnd, (int)cx,(int)cy, (int)(r*0.35f));

            // orange irregular burst
            SDL_SetRenderDrawColor(rnd, 255,110,0,255);
            fillBlob(rnd, (int)cx,(int)cy, r);

            // first 8 debris chunks flying out
            for (int i = 0; i < 8; i++) {
                float reach = r * chunks[i].distMult * 1.9f;
                int   bx    = (int)(cx + cosf(chunks[i].angle) * reach);
                int   by    = (int)(cy + sinf(chunks[i].angle) * reach);
                int   bs    = (int)(chunks[i].sz * 0.6f);
                if (bs < 1) bs = 1;
                SDL_SetRenderDrawColor(rnd, chunks[i].r, chunks[i].g, chunks[i].b, 255);
                SDL_Rect sq = { bx-bs/2, by-bs/2, bs, bs };
                SDL_RenderFillRect(rnd, &sq);
            }
        }

        // ── PHASE 2  (t: 0.25 → 0.58)  full fireball ────────────────────
        else if (t < 0.58f) {
            float p = (t - 0.25f) / 0.33f;
            int   r = (int)(R * (0.55f + 0.45f * p));  // grows to full R

            // dark outer smoke shell
            SDL_SetRenderDrawColor(rnd, 40,33,28, (Uint8)(170 + 70*p));
            fillBlob(rnd, (int)cx,(int)cy, r);

            // deep orange fire
            SDL_SetRenderDrawColor(rnd, 210,65,0,255);
            fillBlob(rnd, (int)cx,(int)cy, (int)(r*0.80f));

            // mid orange
            SDL_SetRenderDrawColor(rnd, 255,155,0,255);
            fillCircle(rnd, (int)cx,(int)cy, (int)(r*0.60f));

            // yellow
            SDL_SetRenderDrawColor(rnd, 255,230,50,255);
            fillCircle(rnd, (int)cx,(int)cy, (int)(r*0.38f));

            // white-hot center
            SDL_SetRenderDrawColor(rnd, 255,255,230,255);
            fillCircle(rnd, (int)cx,(int)cy, (int)(r*0.18f));

            // all 16 debris chunks around the edge
            Uint8 debrisA = (Uint8)(255 * (1.0f - p * 0.5f));
            for (int i = 0; i < 16; i++) {
                float reach = r * chunks[i].distMult * 1.12f;
                int   bx    = (int)(cx + cosf(chunks[i].angle) * reach);
                int   by    = (int)(cy + sinf(chunks[i].angle) * reach);
                int   bs    = chunks[i].sz;
                SDL_SetRenderDrawColor(rnd, chunks[i].r, chunks[i].g, chunks[i].b, debrisA);
                SDL_Rect sq = { bx-bs/2, by-bs/2, bs, bs };
                SDL_RenderFillRect(rnd, &sq);
            }
        }

        // ── PHASE 3  (t: 0.58 → 1.00)  smoke ring + hollow center ───────
        else {
            float p    = (t - 0.58f) / 0.42f;
            int   r    = (int)(R * (1.00f + 0.50f * p));
            int   hole = (int)(r  * (0.38f + 0.28f * p));
            Uint8 a    = (Uint8)(210 * (1.0f - p));

            // outer smoke blob
            SDL_SetRenderDrawColor(rnd, 52,44,36, a);
            fillBlob(rnd, (int)cx,(int)cy, r);

            // hollow center punch-out
            SDL_SetRenderDrawColor(rnd, 0,0,6, a);
            fillCircle(rnd, (int)cx,(int)cy, hole);

            // 6 smoke puffs sitting in the ring band
            float puffA[6] = { 0.00f, 1.05f, 2.09f, 3.14f, 4.19f, 5.24f };
            int   midR     = (r + hole) / 2;
            for (int i = 0; i < 6; i++) {
                int wx = (int)(cx + cosf(puffA[i]) * midR);
                int wy = (int)(cy + sinf(puffA[i]) * midR);
                SDL_SetRenderDrawColor(rnd, 88,76,66, (Uint8)(a*0.7f));
                fillBlob(rnd, wx, wy, (int)(r * 0.24f));
            }
        }

        SDL_SetRenderDrawBlendMode(rnd, SDL_BLENDMODE_NONE);
    }
};
// ===== MAIN =====
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

        // Collision + explosion
        for (auto &bullet : player.getBullets()) {
            if (bullet.isActive()) {
                SDL_Rect br = bullet.getRect();
                if (enemyManager.checkCollision(br)) {
                    bullet.deactivate();
                    explosions.emplace_back(
                        (float)(br.x + br.w / 2),
                        (float)(br.y + br.h / 2)
                    );
                    if (explosionSound)
                        Mix_PlayChannel(-1, explosionSound, 0);
                }
            }
        }

        // Update explosions
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

        for (auto &s : stars) {
            int b = s.brightness;
            SDL_SetRenderDrawColor(renderer, b, b, b, 255);
            SDL_Rect sr = {(int)s.x, (int)s.y, s.size, s.size};
            SDL_RenderFillRect(renderer, &sr);
        }

        player.render(renderer);
        enemyManager.render();

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
