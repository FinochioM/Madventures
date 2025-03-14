#include "entity.h"

Entity::Entity(float x, float y, int width, int height) :
                x(x), y(y), width(width), height(height), textureID("") {
    updateCollider();
}

Entity::~Entity() {
    // Destructor
}

void Entity::update(){
    updateCollider();
}

void Entity::render(Renderer& renderer) {
    if (!textureID.empty()) {
        renderer.renderTexture(textureID, static_cast<int>(x), static_cast<int>(y), width, height);
    }else {
        renderer.setDrawColor(255, 0, 255, 255);
        renderer.fillRect(static_cast<int>(x), static_cast<int>(y), width, height);
    }
}

void Entity::updateCollider() {
    collider.x = static_cast<int>(x);
    collider.y = static_cast<int>(y);
    collider.w = width;
    collider.h = height;
}

SDL_Rect Entity::getCollider() const {
    return collider;
}