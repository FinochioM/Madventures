#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "renderer.h"
#include "entity.h"
#include "player.h"
#include "tilemap.h"
#include "map_editor.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL_opengl.h>

enum class GameState {
    CITY,
    ARENA,
    EDITOR
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
    void switchToEditor();

    bool loadAssets(Renderer& renderer);

    TileMap* getTileMap() const { return tileMap; }
    SDL_Window* getWindow() const { return window; }
    SDL_GLContext getGLContext() const { return glContext; }

private:
    GameState currentState;
    bool isRunning;
    bool playerSelected;

    int mouseX, mouseY;

    int score;
    int movesRemaining;

    SDL_Rect arenaButton;
    SDL_Rect editorButton;

    Player* player;
    std::vector<Entity*> entities;

    TileMap* tileMap;
    MapEditor* mapEditor;

    SDL_Window* window;
    SDL_GLContext glContext;
    bool imguiInitialized;

    void handleCityEvents(SDL_Event& e);
    void handleArenaEvents(SDL_Event& e);
    void handleEditorEvents(SDL_Event& e);
    void updateCity();
    void updateArena();
    void updateEditor();
    void renderCity(Renderer& renderer);
    void renderArena(Renderer& renderer);
    void renderEditor(Renderer& renderer);

    void renderMovementRange(Renderer& renderer);
    void placePlayerInValidPosition();

    std::string currentCity;
    std::string currentArena;

    bool loadMap(const std::string& mapName);
    void setCurrentCity(const std::string& cityName);
    void setCurrentArena(const std::string& arenaName);

    bool initializeImGui();
    void shutdownImGui();
};

#endif // GAME_H