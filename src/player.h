#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <SDL2/SDL.h>
#include <vector>

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

    void setPath(const std::vector<std::pair<int, int>>& newPath);
    bool hasPath() const { return !path.empty() && currentPathIndex < path.size(); }

    bool isCurrentlyMoving() const { return hasTarget; }

private:
    int health;
    int speed;
    int direction;
    bool isMoving;
    float targetX, targetY;
    bool hasTarget;
    bool selected;

    std::vector<std::pair<int, int>> path;
    size_t currentPathIndex;
};

#endif // PLAYER_H