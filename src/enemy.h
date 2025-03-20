#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"
#include "player.h"
#include "tilemap.h"
#include <string>

class Enemy : public Entity {
public:
    Enemy(float x, float y, int health = 10, int damage = 2);
    ~Enemy();

    void update() override;
    void render(Renderer& renderer) override;

    bool takeDamage(int amount);
    int getHealth() const { return health; }
    bool isDead() const { return health <= 0; }
    int getDamage() const { return damage; }

    void setTargetPosition(float targetX, float targetY);
    bool isCurrentlyMoving() const { return hasTarget; }

    void calculateAttackTargets(const TileMap* tileMap, Player* player);
    bool canAttackPlayer() const { return inAttackRange; }

    void setTargeted(bool targeted) { isTargeted = targeted; }
    bool isBeingTargeted() const { return isTargeted; }

private:
    int health;
    int damage;
    int movementSpeed;
    int attackRange;
    bool inAttackRange;
    bool isTargeted;

    float targetX, targetY;
    bool hasTarget;
    std::vector<std::pair<int, int>> path;
    size_t currentPathIndex;

    void calculatePath(const TileMap* tileMap, int startX, int startY, int endX, int endY);
};

#endif // ENEMY_H