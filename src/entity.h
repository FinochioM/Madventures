#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL.h>
#include <string>
#include "renderer.h"

class Entity {
public:
    Entity(float x, float y, int width, int height);
    virtual ~Entity();

    virtual void update();
    virtual void render(Renderer& renderer);

    SDL_Rect getCollider() const;

    float getX() const { return x; }
    float getY() const { return y; }
    void setX(float newX) { x = newX; updateCollider(); }
    void setY(float newY) { y = newY; updateCollider(); }

protected:
    float x, y;
    int width, height;
    SDL_Rect collider;
    std::string textureID;

    void updateCollider();
};

#endif // ENTITY_H