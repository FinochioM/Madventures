#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "renderer.h"
#include "entity.h"
#include "player.h"
#include "tilemap.h"

enum class GameState {
    CITY,
    ARENA
};

class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void handleEvent(SDL_Event& e);
    void update();
    void render(Renderer& renderer);
    void cleanup();

    void switchToCity();
    void switchToArena();

    bool loadAssets(Renderer& renderer);

    TileMap* getTileMap() const { return tileMap; }

private:
    GameState currentState;
    bool isRunning;
    bool playerSelected;

    int mouseX, mouseY;

    int score;
    int movesRemaining;

    SDL_Rect arenaButton;

    Player* player;
    std::vector<Entity*> entities;

    TileMap* tileMap;

    void handleCityEvents(SDL_Event& e);
    void handleArenaEvents(SDL_Event& e);
    void updateCity();
    void updateArena();
    void renderCity(Renderer& renderer);
    void renderArena(Renderer& renderer);
};

#endif // GAME_H