
#include "EnemyManager.h"
#include "SlowEnemy.h"
#include "FastEnemy.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>

using namespace std;

EnemyManager::EnemyManager(SDL_Renderer* renderer, int width, int height, std::vector<SDL_Texture*> textures) {
    this->renderer = renderer;
    this->textures = textures;
    windowWidth = width;
    windowHeight = height;
    spawnTimer = 0.0f;
    spawnInterval = 0.8f;
    points = 0;
    isGameOver = false;
}

void EnemyManager::update(float deltaTime) {
    spawnTimer += deltaTime;

    if (spawnTimer >= spawnInterval) {
        int enemySize = 70;
        int gap = 15;
        int totalHeight = 5 * enemySize + 3 * gap;
        int startY = (windowHeight - totalHeight) / 2;

        for (int i = 0; i < 5; i++) {
            int x = windowWidth;
            int y = startY + i * (enemySize + gap);
            SDL_Texture* tex = textures[i % textures.size()];

            if (spawnSlow) {
                enemies.push_back(std::make_unique<SlowEnemy>(x, y, tex));
            } else {
                enemies.push_back(std::make_unique<FastEnemy>(x, y, tex));
            }
        }

        spawnSlow = !spawnSlow;
        spawnTimer = 0.0f;
        spawnInterval = 3.0f;
    }

    for (auto& enemy : enemies) {
        enemy->update(deltaTime);
        if (enemy->getRect().x < 0) {
            isGameOver = true;
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return e->isOffScreen(); }),
        enemies.end()
    );
}

void EnemyManager::render() {
    for (auto& enemy : enemies)
        enemy->render(renderer);
}

bool EnemyManager::checkCollision(SDL_Rect bulletRect) {
    for (auto it = enemies.begin(); it != enemies.end(); ++it) {
        SDL_Rect enemyRect = (*it)->getRect();

        if (bulletRect.x < enemyRect.x + enemyRect.w &&
            bulletRect.x + bulletRect.w > enemyRect.x &&
            bulletRect.y < enemyRect.y + enemyRect.h &&
            bulletRect.y + bulletRect.h > enemyRect.y)
        {
            enemies.erase(it);
            points++;
            return true;
        }
    }
    return false;
}

