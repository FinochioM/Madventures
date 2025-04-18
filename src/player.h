#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <SDL2/SDL.h>
#include <vector>
#include "tilemap.h"

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

    void calculateAvailableTiles(const TileMap* tileMap);
    const std::vector<std::pair<int, int>>& getAvailableTiles() const { return availableTiles; }
    bool isTileAvailable(int gridX, int gridY) const;
    int getMovementRange() const { return movementRange; }
    void setMovementRange(int range) { movementRange = range; }

    int getAttackDamage() const { return attackDamage; }
    void setAttackDamage(int damage) { attackDamage = damage; }

    int getAttackRange() const { return attackRange; }
    void setAttackRange(int range) { attackRange = range; }

    int getMaxAttacks() const { return maxAttacks; }
    void setMaxAttacks(int attacks) { maxAttacks = attacks; }

    int getRemainingAttacks() const { return remainingAttacks; }
    void setRemainingAttacks(int remain) { remainingAttacks = remain; }

    void useAttack() { if (remainingAttacks > 0) remainingAttacks--; }
    bool hasAttacksRemaining() const { return remainingAttacks > 0; }

    void calculateAttackTargets(const TileMap* tileMap);
    const std::vector<std::pair<int, int>>& getAttackTargets() const { return attackTargets; }
    bool isTileInAttackRange(int griX, int gridY) const;
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

    int movementRange;
    std::vector<std::pair<int, int>> availableTiles;

    int attackDamage;
    int attackRange;
    int maxAttacks;
    int remainingAttacks;
    std::vector<std::pair<int, int>> attackTargets;
};

#endif // PLAYER_H