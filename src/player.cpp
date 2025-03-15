#include "player.h"
#include <cmath>

Player::Player(float x, float y)
    : Entity(std::floor(x / 32) * 32, std::floor(y / 32) * 32, 32, 32), health(100), speed(4), isMoving(false), direction(0),
                            targetX(x), targetY(y), hasTarget(false), selected(false), currentPathIndex(0) {
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
                targetX = path[currentPathIndex].first * tileSize + tileSize / 2;
                targetY = path[currentPathIndex].second * tileSize + tileSize / 2;
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
        targetX = path[0].first * tileSize + tileSize / 2;
        targetY = path[0].second * tileSize + tileSize / 2;
        hasTarget = true;
    }
}