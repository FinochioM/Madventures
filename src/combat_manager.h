#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#include <vector>
#include "player.h"
#include "enemy.h"
#include "tilemap.h"
#include "renderer.h"

class CombatManager {
public:
    CombatManager(Player* player, TileMap* tileMap);
    ~CombatManager();

    void update();
    void render(Renderer& renderer);

    void startCombat(int initialWave = 1);
    bool isInCombat() const { return inCombat; }
    void endCombat();

    void playerAttack(int targetIndex);
    bool canAttack(int gridX, int gridY) const;

    void spawnWave();
    bool isWaveComplete() const { return enemies.empty(); }
    int getCurrentWave() const { return currentWave; }

    const std::vector<Enemy*>& getEnemies() const { return enemies; }
    int getEnemyAt(int gridX, int gridY) const;

    void handleCombatEvent(int gridX, int gridY);

private:
    Player* player;
    TileMap* tileMap;
    std::vector<Enemy*> enemies;

    bool inCombat;
    int currentWave;
    int enemiesPerWave;
    int maxWaves;

    bool playerTurn;

    void executeEnemyTurns();
    void cleanupDeadEnemies();
    void checkCombatState();

    std::vector<std::pair<int, int>> getValidSpawnPositions() const;
};

#endif // COMBAT_MANAGER_H