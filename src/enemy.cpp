#include "enemy.h"
#include <cmath>
#include <iostream>

Enemy::Enemy(float x, float y, int health, int damage)
    : Entity(std::floor(x / 32) * 32, std::floor(y / 32) * 32, 32, 32),
      health(health), damage(damage), movementSpeed(3), attackRange(1),
      inAttackRange(false), isTargeted(false),
      targetX(x), targetY(y), hasTarget(false), currentPathIndex(0) {
    textureID = "enemy";
}

Enemy::~Enemy() {
    // Destructor
}

void Enemy::update() {
    if (hasTarget) {
        float dx = targetX - x;
        float dy = targetY - y;
        float distance = std::sqrt(dx*dx + dy*dy);

        if (distance > movementSpeed) {
            float moveX = (dx / distance) * movementSpeed;
            float moveY = (dy / distance) * movementSpeed;

            setX(x + moveX);
            setY(y + moveY);
        } else {
            setX(targetX);
            setY(targetY);

            if (!path.empty() && currentPathIndex < path.size() - 1) {
                currentPathIndex++;
                int tileSize = 32;
                targetX = path[currentPathIndex].first * tileSize + tileSize - 32;
                targetY = path[currentPathIndex].second * tileSize + tileSize - 32;
            } else {
                hasTarget = false;
                path.clear();
            }
        }
    }

    Entity::update();
}

void Enemy::render(Renderer& renderer) {
    Entity::render(renderer);

    if (isTargeted) {
        renderer.renderTexture("tile_selection_enemy",
                              static_cast<int>(x) - 2,
                              static_cast<int>(y) - 2,
                              width + 4, height + 4);
    }


    std::string healthText = "HP " + std::to_string(health);
    renderer.drawText(healthText, static_cast<int>(x), static_cast<int>(y - 20));
}

bool Enemy::takeDamage(int amount) {
    health -= amount;
    return health <= 0;
}

void Enemy::setTargetPosition(float targetX, float targetY) {
    this->targetX = std::floor(targetX / 32) * 32;
    this->targetY = std::floor(targetY / 32) * 32;
    hasTarget = true;
}

void Enemy::calculateAttackTargets(const TileMap* tileMap, Player* player) {
    if (!player) return;

    int enemyGridX, enemyGridY;
    tileMap->pixelToGrid(x, y, enemyGridX, enemyGridY);

    int playerGridX, playerGridY;
    tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

    int distance = std::abs(playerGridX - enemyGridX) + std::abs(playerGridY - enemyGridY);

    inAttackRange = (distance <= attackRange);
}

void Enemy::calculatePath(const TileMap* tileMap, int startX, int startY, int endX, int endY) {
    path = tileMap->findPath(startX, startY, endX, endY);

    if (!path.empty()) {
        currentPathIndex = 0;
        int tileSize = 32;
        targetX = path[0].first * tileSize + tileSize - 32;
        targetY = path[0].second * tileSize + tileSize - 32;
        hasTarget = true;
    }
}