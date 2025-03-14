#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <SDL2/SDL.h>

class Player : public Entity {
public:
    Player(float x, float y);
    ~Player();

    void update() override;
    void render(Renderer& renderer) override;

    void setTargetPosition(float targetX, float targetY);
    bool isPointOnPlayer(int pointX, int pointY) const;
    bool isSelected() const { return selected; }
    void setSelected(bool select) { selected = select; }

    int getHealth() const { return health; }
    void setHealth(int newHealth) { health = newHealth; }

private:
    int health;
    int speed;
    bool isMoving;
    int direction;

    float targetX, targetY;
    bool hasTarget;
    bool selected;
};

#endif // PLAYER_H