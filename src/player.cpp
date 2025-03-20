#include "player.h"
#include "tilemap.h"
#include <cmath>
#include <queue>
#include <set>
#include <algorithm>
#include <iostream>

Player::Player(float x, float y)
    : Entity(std::floor(x / 32) * 32, std::floor(y / 32) * 32, 32, 32), health(100), speed(5), isMoving(false), direction(0),
                            targetX(x), targetY(y), hasTarget(false), selected(false), currentPathIndex(0), movementRange(5),
                            attackDamage(10), attackRange(1), maxAttacks(5), remainingAttacks(5) {
    textureID = "player";
}

Player::~Player() {
    // Destructor
}

void Player::update() {
    if (hasTarget) {
        float dx = targetX - x;
        float dy = targetY - y;
        float distance = std::sqrt(dx*dx + dy*dy);

        if (distance > speed) {
            float moveX = (dx / distance) * speed;
            float moveY = (dy / distance) * speed;

            setX(x + moveX);
            setY(y + moveY);

            if (std::abs(dx) > std::abs(dy)) {
                direction = (dx > 0) ? 2 : 1;
            } else {
                direction = (dy > 0) ? 0 : 3;
            }

            isMoving = true;
        } else {
            setX(targetX);
            setY(targetY);

            if (!path.empty() && currentPathIndex < path.size() - 1) {
                currentPathIndex++;
                int tileSize = 32; // again get this from tilemap?
                targetX = path[currentPathIndex].first * tileSize + tileSize - 32;
                targetY = path[currentPathIndex].second * tileSize + tileSize - 32;
            } else {
                isMoving = false;
                hasTarget = false;
                path.clear();
            }
        }
    }

    Entity::update();
}

void Player::render(Renderer& renderer) {
    if (selected) {
        renderer.setDrawColor(0, 255, 0, 255);
        SDL_Rect selectionRect = {
            static_cast<int>(x) - 2,
            static_cast<int>(y) - 2,
            width + 4,
            height + 4
        };
        renderer.drawRect(selectionRect);
    }

    Entity::render(renderer);

    std::string healthText = "HP " + std::to_string(health);
    renderer.drawText(healthText, static_cast<int>(x), static_cast<int>(y - 20));

    std::string attacksText = "Attacks: " + std::to_string(remainingAttacks);
    renderer.drawText(attacksText, static_cast<int>(x), static_cast<int>(y - 40));
}

void Player::setTargetPosition(float targetX, float targetY) {
    this->targetX = std::floor(targetX / 32) * 32;
    this->targetY = std::floor(targetY / 32) * 32;
    hasTarget = true;
}

bool Player::isPointOnPlayer(int pointX, int pointY) const {
    return (pointX >= x && pointX <= x +width &&
            pointY >= y && pointY <= y + height);
}

void Player::setPath(const std::vector<std::pair<int, int>>& newPath) {
    path = newPath;
    currentPathIndex = 0;

    if (!path.empty()) {
        int tileSize = 32; // later get this from tilemap?
        targetX = path[0].first * tileSize + tileSize - 32;
        targetY = path[0].second * tileSize + tileSize - 32;
        hasTarget = true;
    }
}

void Player::calculateAvailableTiles(const TileMap* tileMap) {
    availableTiles.clear();

    int playerGridX, playerGridY;
    tileMap->pixelToGrid(x, y, playerGridX, playerGridY);

    std::queue<std::pair<std::pair<int, int>, int>> que;
    std::set<std::pair<int, int>> visited;

    que.push(std::make_pair(std::make_pair(playerGridX, playerGridY), 0));
    visited.insert(std::make_pair(playerGridX, playerGridY));
    availableTiles.push_back(std::make_pair(playerGridX, playerGridY));

    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};

    while (!que.empty()) {
        std::pair<int, int> current = que.front().first;
        int distance = que.front().second;
        que.pop();

        if (distance >= movementRange) continue;

        for (int i = 0; i < 4; i++) {
            int newX = current.first + dx[i];
            int newY = current.second + dy[i];
            std::pair<int, int> newPos(newX, newY);

            if (visited.find(newPos) != visited.end() || !tileMap->isWalkable(newX, newY)) {
                continue;
            }

            visited.insert(newPos);
            availableTiles.push_back(newPos);
            que.push(std::make_pair(newPos, distance + 1));
        }
    }
}

bool Player::isTileAvailable(int gridX, int gridY) const {
    return std::find(availableTiles.begin(), availableTiles.end(),
                    std::make_pair(gridX, gridY)) != availableTiles.end();
}

void Player::calculateAttackTargets(const TileMap* tileMap) {
    attackTargets.clear();

    int playerGridX, playerGridY;
    tileMap->pixelToGrid(x, y, playerGridX, playerGridY);

    std::cout << "Calculating attack targets from player position: "
              << playerGridX << "," << playerGridY << std::endl;

    for (int dx = -attackRange; dx <= attackRange; dx++) {
        for (int dy = -attackRange; dy <= attackRange; dy++) {
            if (std::abs(dx) + std::abs(dy) > attackRange) continue;

            int targetX = playerGridX + dx;
            int targetY = playerGridY + dy;

            if (dx == 0 && dy == 0) continue;

            if (tileMap->isValidGridPosition(targetX, targetY)) {
                attackTargets.push_back(std::make_pair(targetX, targetY));
                std::cout << "Added attack target: " << targetX << "," << targetY << std::endl;
            }
        }
    }

    std::cout << "Total attack targets: " << attackTargets.size() << std::endl;
}

bool Player::isTileInAttackRange(int gridX, int gridY) const {
    // Debug output to trace attack range calculations
    std::cout << "Checking if tile " << gridX << "," << gridY << " is in attack range..." << std::endl;

    for (const auto& tile : attackTargets) {
        std::cout << "Attack target: " << tile.first << "," << tile.second << std::endl;
        if (tile.first == gridX && tile.second == gridY) {
            std::cout << "Target is in attack range!" << std::endl;
            return true;
        }
    }

    std::cout << "Target is NOT in attack range" << std::endl;
    return false;
}