#include "game.h"
#include <iostream>
#include <fstream>
#include <filesystem>

Game::Game() : currentState(GameState::CITY), isRunning(true), mouseX(0), mouseY(0), score(0), movesRemaining(10), playerSelected(false) {
    arenaButton = {350, 400, 100, 50};
    editorButton = {460, 400, 100, 50};

    tileMap = new TileMap(32, 800, 600);
    mapEditor = new MapEditor(tileMap);

    player = new Player(0, 0);
}

Game::~Game() {
    delete player;
    delete tileMap;
    delete mapEditor;

    for (auto entity : entities) {
        delete entity;
    }

    entities.clear();
}

bool Game::initialize() {
    tileMap->initialize();

    currentCity = "city";
    currentArena = "arena";

    bool cityLoaded = loadMap(currentCity);
    if (!cityLoaded) {
        std::cout << "No city map found. Use the editor to create one." << std::endl;

        std::filesystem::create_directories("maps");

        switchToEditor();
        mapEditor->saveMap("maps/city.json");
        mapEditor->saveMap("maps/arena.json");

        loadMap(currentCity);
    }

    placePlayerInValidPosition();
    return true;
}

void Game::placePlayerInValidPosition() {
    for (int y = 0; y < tileMap->getGridHeight(); y++) {
        for (int x = 0; x < tileMap->getGridWidth(); x++) {
            if (tileMap->isWalkable(x, y)) {
                int pixelX, pixelY;
                tileMap->gridToPixel(x, y, pixelX, pixelY);

                pixelX += tileMap->getTileSize() / 2 - player->getCollider().w / 2;
                pixelY += tileMap->getTileSize() / 2 - player->getCollider().h / 2;

                player->setX(pixelX);
                player->setY(pixelY);
                return;
            }
        }
    }

    player->setX(tileMap->getTileSize());
    player->setY(tileMap->getTileSize());
    std::cout << "Warning: No walkable tiles found for player placement." << std::endl;
}

bool Game::loadAssets(Renderer& renderer) {
    if (!renderer.loadTexture("player", "assets/player.png")) {
        std::cout << "Warning: Player sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("tile_grass", "assets/tile_grass.png")){
        std::cout << "Warning: Tile Grass sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("tile_wall", "assets/tile_wall.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    return true;
}

void Game::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouseX, &mouseY);
    }

    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
        if (currentState == GameState::EDITOR) {
            switchToCity();
        } else {
            switchToEditor();
        }
    }

    switch (currentState) {
        case GameState::CITY:
            handleCityEvents(e);
            break;
        case GameState::ARENA:
            handleArenaEvents(e);
            break;
        case GameState::EDITOR:
            handleEditorEvents(e);
            break;
    }
}

void Game::handleCityEvents(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            if (mouseX >= arenaButton.x && mouseX <= arenaButton.x + arenaButton.w &&
                mouseY >= arenaButton.y && mouseY <= arenaButton.y + arenaButton.h) {
                    switchToArena();
                    return;
            }

            if (mouseX >= editorButton.x && mouseX <= editorButton.x + editorButton.w &&
                mouseY >= editorButton.y && mouseY <= editorButton.y + editorButton.h) {
                    switchToEditor();
                    return;
            }

            if (player->isPointOnPlayer(mouseX, mouseY)) {
                player->setSelected(true);
                playerSelected = true;

                player->calculateAvailableTiles(tileMap);
            }else if (playerSelected && !player->isCurrentlyMoving()) {
                int targetGridX, targetGridY;
                tileMap->pixelToGrid(mouseX, mouseY, targetGridX, targetGridY);

                if (tileMap->isWalkable(targetGridX, targetGridY) &&
                    player->isTileAvailable(targetGridX, targetGridY)) {
                    int playerGridX, playerGridY;
                    tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

                    std::vector<std::pair<int, int>> path =
                        tileMap->findPath(playerGridX, playerGridY, targetGridX, targetGridY);

                    if (!path.empty()) {
                        player->setPath(path);
                        player->setSelected(false);
                        playerSelected = false;
                    } else {
                        std::cout << "No valid path found?" << std::endl;
                    }
                } else {
                    std::cout << "Cannot move there - out of range or not walkable" << std::endl;
                }
            }
        }
    }
}

void Game::handleArenaEvents(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            if (movesRemaining > 0) {
                movesRemaining--;
                score += 10;
            }

            if (movesRemaining <= 0) {
                switchToCity();
            }
        }
    }
}

void Game::handleEditorEvents(SDL_Event& e) {
    mapEditor->handleEvent(e);
}

void Game::update() {
    player->update();

    for (auto entity : entities) {
        entity->update();
    }

    switch (currentState) {
        case GameState::CITY:
            updateCity();
            break;
        case GameState::ARENA:
            updateArena();
            break;
        case GameState::EDITOR:
            updateEditor();
            break;
    }
}

void Game::updateCity() {
    player->update();

    for (auto entity : entities) {
        entity->update();
    }
}

void Game::updateArena() {
    player->update();

    for (auto entity : entities) {
        entity->update();
    }
}

void Game::updateEditor() {
    mapEditor->update();
}

void Game::render(Renderer& renderer) {
    switch (currentState) {
        case GameState::CITY:
            renderCity(renderer);
            tileMap->render(renderer);
            renderMovementRange(renderer);
            for (auto entity : entities) {
                entity->render(renderer);
            }
            player->render(renderer);
            break;
        case GameState::ARENA:
            renderArena(renderer);
            tileMap->render(renderer);
            renderMovementRange(renderer);
            for (auto entity : entities) {
                entity->render(renderer);
            }
            player->render(renderer);
            break;
        case GameState::EDITOR:
            renderEditor(renderer);
            break;
    }
}

void Game::renderCity(Renderer& renderer) {
    renderer.setDrawColor(100, 100, 200, 255);
    renderer.fillRect(0, 0, 800, 600);

    renderer.setDrawColor(150, 150, 150, 255);
    renderer.fillRect(arenaButton);

    renderer.setDrawColor(0, 0, 0, 255); // Black text
    renderer.drawText("Arena", arenaButton.x + 25, arenaButton.y + 15);

    std::string scoreText = "Score: " + std::to_string(score);
    renderer.drawText(scoreText, 20, 20);
}

void Game::renderArena(Renderer& renderer) {
    renderer.setDrawColor(200, 100, 100, 255);
    renderer.fillRect(0, 0, 800, 600);

    std::string scoreText = "Score: " + std::to_string(score);
    renderer.drawText(scoreText, 20, 20);

    std::string movesText = "Moves: " + std::to_string(movesRemaining);
    renderer.drawText(movesText, 20, 50);

    renderer.drawText("Click to use a move and earn points", 250, 300);
}

void Game::renderEditor(Renderer& renderer) {
    renderer.setDrawColor(100, 100, 100, 255);
    renderer.fillRect(0, 0, 800, 600);

    tileMap->render(renderer);
    mapEditor->render(renderer);

    renderer.drawText("Map Editor - Press M to return to game", 10, 10);
    renderer.drawText("Press Ctrl+S to save map", 10, 30);
    renderer.drawText("Press Ctrl+L to load map", 10, 50);
}

void Game::switchToCity() {
    currentState = GameState::CITY;
    mapEditor->setActive(false);
    loadMap(currentCity);
    placePlayerInValidPosition();
    std::cout << "Switched to city: " << currentCity << std::endl;
}

void Game::switchToArena() {
    currentState = GameState::ARENA;
    mapEditor->setActive(false);
    loadMap(currentArena);
    placePlayerInValidPosition();
    movesRemaining = 10;
    std::cout << "Switched to arena: " << currentArena << std::endl;
}

void Game::switchToEditor() {
    currentState = GameState::EDITOR;
    mapEditor->setActive(true);

    std::ifstream mapCheck("map.json");
    if (!mapCheck.good()) {
        mapEditor->saveMap("map.json");
        std::cout << "Created default map file." << std::endl;
    }
    mapCheck.close();

    std::cout << "Switched to editor" << std::endl;
}

void Game::cleanup() {
    // Cleanup I guesss.
}

void Game::renderMovementRange(Renderer& renderer) {
    if (!playerSelected) return;

    const auto& availableTiles = player->getAvailableTiles();

    for (const auto& tile : availableTiles) {
        int pixelX, pixelY;
        tileMap->gridToPixel(tile.first, tile.second, pixelX, pixelY);

        renderer.setDrawColor(0, 255, 0, 255);
        SDL_Rect tileRect = {pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize()};
        renderer.drawRect(tileRect);
    }
}

bool Game::loadMap(const std::string& mapName) {
    std::string mapPath = "maps/" + mapName + ".json";
    if (!mapEditor->loadMap(mapPath)) {
        std::cerr << "Failed to load map: " << mapPath << std::endl;
        return false;
    }

    return true;
}

void Game::setCurrentCity(const std::string& cityName) {
    currentCity = cityName;
    if (currentState == GameState::CITY) {
        loadMap(currentCity);
    }
}

void Game::setCurrentArena(const std::string& arenaName) {
    currentArena = arenaName;
    if (currentState == GameState::ARENA) {
        loadMap(currentArena);
    }
}