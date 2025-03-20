#include "combat_manager.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

CombatManager::CombatManager(Player* player, TileMap* tileMap)
    : player(player), tileMap(tileMap), inCombat(false), currentWave(0),
      enemiesPerWave(2), maxWaves(5), playerTurn(true) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

CombatManager::~CombatManager() {
    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
}

void CombatManager::update() {
    if (!inCombat) return;

    for (auto enemy : enemies) {
        enemy->update();
    }

    cleanupDeadEnemies();

    if (isWaveComplete()) {
        if (currentWave < maxWaves) {
            currentWave++;
            spawnWave();
        } else {
            endCombat();
        }
    }

    if (!player->hasAttacksRemaining()) {
        endCombat();
    }

    if (!playerTurn) {
        executeEnemyTurns();
        playerTurn = true;
    }
}

void CombatManager::render(Renderer& renderer) {
    if (!inCombat) return;

    for (auto enemy : enemies) {
        enemy->render(renderer);
    }

    std::string waveText = "Wave: " + std::to_string(currentWave) + "/" + std::to_string(maxWaves);
    renderer.drawText(waveText, 10, 10);
}

void CombatManager::startCombat(int initialWave) {
    inCombat = true;
    currentWave = initialWave;
    playerTurn = true;

    player->setRemainingAttacks(player->getMaxAttacks());

    spawnWave();

    std::cout << "Combat started - Wave " << currentWave << std::endl;
}

void CombatManager::endCombat() {
    inCombat = false;

    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    std::cout << "Combat ended - Returning to city" << std::endl;
}

void CombatManager::playerAttack(int targetIndex) {
    if (!inCombat || !playerTurn || targetIndex < 0 || targetIndex >= enemies.size()) return;

    Enemy* target = enemies[targetIndex];

    bool killed = target->takeDamage(player->getAttackDamage());

    std::cout << "Player attacked enemy for " << player->getAttackDamage() << " damage" << std::endl;

    player->useAttack();

    playerTurn = false;
}

bool CombatManager::canAttack(int gridX, int gridY) const {
    return player->isTileInAttackRange(gridX, gridY) && getEnemyAt(gridX, gridY) != -1;
}

void CombatManager::spawnWave() {
    for (auto enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    auto spawnPositions = getValidSpawnPositions();
    if (spawnPositions.empty()) {
        std::cout << "Warning: No valid spawn positions found!" << std::endl;
        return;
    }

    int numEnemies = enemiesPerWave + (currentWave - 1);

    numEnemies = std::min(numEnemies, static_cast<int>(spawnPositions.size()));

    for (int i = 0; i < numEnemies; i++) {
        int index = std::rand() % spawnPositions.size();
        auto pos = spawnPositions[index];
        spawnPositions.erase(spawnPositions.begin() + index);

        int tileSize = tileMap->getTileSize();
        float spawnX = pos.first * tileSize;
        float spawnY = pos.second * tileSize;

        int enemyHealth = 10 + (currentWave * 5);
        int enemyDamage = 2 + currentWave;

        Enemy* enemy = new Enemy(spawnX, spawnY, enemyHealth, enemyDamage);
        enemies.push_back(enemy);
    }

    std::cout << "Wave " << currentWave << " spawned with " << numEnemies << " enemies" << std::endl;
}

std::vector<std::pair<int, int>> CombatManager::getValidSpawnPositions() const {
    std::vector<std::pair<int, int>> validPositions;

    int playerGridX, playerGridY;
    tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

    const int minDistanceFromPlayer = 3;

    for (int y = 0; y < tileMap->getGridHeight(); y++) {
        for (int x = 0; x < tileMap->getGridWidth(); x++) {
            if (tileMap->isWalkable(x, y)) {
                int distance = std::abs(x - playerGridX) + std::abs(y - playerGridY);

                if (distance >= minDistanceFromPlayer) {
                    validPositions.push_back(std::make_pair(x, y));
                }
            }
        }
    }

    return validPositions;
}

void CombatManager::executeEnemyTurns() {
    if (enemies.empty()) return;

    for (auto enemy : enemies) {
        enemy->calculateAttackTargets(tileMap, player);

        if (enemy->canAttackPlayer()) {
            player->setHealth(player->getHealth() - enemy->getDamage());
            std::cout << "Enemy attacked player for " << enemy->getDamage() << " damage" << std::endl;
        } else {
            if (!enemy->isCurrentlyMoving()) {
                int enemyGridX, enemyGridY;
                tileMap->pixelToGrid(enemy->getX(), enemy->getY(), enemyGridX, enemyGridY);

                int playerGridX, playerGridY;
                tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

                // Find path to player and move along it
                auto path = tileMap->findPath(enemyGridX, enemyGridY, playerGridX, playerGridY);
                if (!path.empty() && path.size() > 1) {
                    // Move to the first step in the path (skip starting position)
                    int tileSize = tileMap->getTileSize();
                    float targetX = path[1].first * tileSize;
                    float targetY = path[1].second * tileSize;
                    enemy->setTargetPosition(targetX, targetY);
                }
            }
        }
    }
}

void CombatManager::cleanupDeadEnemies() {
    auto it = enemies.begin();
    while (it != enemies.end()) {
        if ((*it)->isDead()) {
            std::cout << "Enemy defeated!" << std::endl;
            delete *it;
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void CombatManager::handleCombatEvent(int gridX, int gridY) {
    if (!inCombat || !playerTurn) return;

    // Check if player is attacking an enemy
    int enemyIndex = getEnemyAt(gridX, gridY);
    if (enemyIndex != -1 && canAttack(gridX, gridY)) {
        playerAttack(enemyIndex);
    }
}

int CombatManager::getEnemyAt(int gridX, int gridY) const {
    for (size_t i = 0; i < enemies.size(); i++) {
        int enemyGridX, enemyGridY;
        tileMap->pixelToGrid(enemies[i]->getX(), enemies[i]->getY(), enemyGridX, enemyGridY);

        if (enemyGridX == gridX && enemyGridY == gridY) {
            return static_cast<int>(i);
        }
    }

    return -1;
}