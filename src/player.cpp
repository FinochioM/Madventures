#include "player.h"
#include <cmath>

Player::Player(float x, float y)
    : Entity(std::floor(x / 32) * 32, std::floor(y / 32) * 32, 32, 32), health(100), speed(4), isMoving(false), direction(0),
                            targetX(x), targetY(y), hasTarget(false), selected(false) {
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

        if (std::abs(dx) > 1.0f) {
            float moveX = (dx > 0) ? speed : -speed;
            setX(x + moveX);
            direction = (dx > 0) ? 2 : 1;
        }else if (std::abs(dy) > 1.0f) {
            float moveY = (dy > 0) ? speed : -speed;
            setY(y + moveY);
            direction = (dy > 0) ? 0 : 3;
        }else {
            setX(targetX);
            setY(targetY);
            isMoving = false;
            hasTarget = false;
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