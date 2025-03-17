#include "game.h"
#include <iostream>


Game::Game() : currentState(GameState::CITY), isRunning(true), mouseX(0), mouseY(0), score(0), movesRemaining(10), playerSelected(false) {
    //arenaButton = {350, 400, 100, 50};

    tileMap = new TileMap(32, 800, 600);

    player = new Player(0, 0);
}

Game::~Game() {
    delete player;
    delete tileMap;

    for (auto entity : entities) {
        delete entity;
    }

    entities.clear();
}

bool Game::initialize() {
    tileMap->initialize();

    int playerGridX, playerGridY;
    tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

    if (!tileMap->isWalkable(playerGridX, playerGridY)) {
        for (int y = 0; y < tileMap->getGridHeight(); y++) {
            for (int x = 0; x < tileMap->getGridWidth(); x++) {
                if (tileMap->isWalkable(x, y)) {
                    int pixelX, pixelY;
                    tileMap->gridToPixel(x, y, pixelX, pixelY);

                    int halfTileSize = tileMap->getTileSize() / 2;
                    pixelX += halfTileSize;
                    pixelY += halfTileSize;

                    player->setX(pixelX - player->getCollider().w / 2);
                    player->setY(pixelY - player->getCollider().h / 2);
                    break;
                }
            }
        }
    }

    return true;
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

    switch (currentState) {
        case GameState::CITY:
            handleCityEvents(e);
            break;
        case GameState::ARENA:
            handleArenaEvents(e);
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
    }
}

void Game::updateCity() {
    // City update logic
}

void Game::updateArena() {
    // Arena update logic
}

void Game::render(Renderer& renderer) {
    switch (currentState) {
        case GameState::CITY:
            renderCity(renderer);
            break;
        case GameState::ARENA:
            renderArena(renderer);
            break;
    }

    tileMap->render(renderer);

    renderMovementRange(renderer);

    for (auto entity : entities) {
        entity->render(renderer);
    }

    player->render(renderer);
}

void Game::renderCity(Renderer& renderer) {
    renderer.setDrawColor(100, 100, 200, 255); // Light blue background for city
    renderer.fillRect(0, 0, 800, 600);

    renderer.setDrawColor(150, 150, 150, 255); // Gray button
    renderer.fillRect(arenaButton);

    renderer.setDrawColor(0, 0, 0, 255); // Black text
    renderer.drawText("Arena", arenaButton.x + 25, arenaButton.y + 15);

    std::string scoreText = "Score: " + std::to_string(score);
    renderer.drawText(scoreText, 20, 20);
}

void Game::renderArena(Renderer& renderer) {
    renderer.setDrawColor(200, 100, 100, 255); // Light red background for arena
    renderer.fillRect(0, 0, 800, 600);

    std::string scoreText = "Score: " + std::to_string(score);
    renderer.drawText(scoreText, 20, 20);

    std::string movesText = "Moves: " + std::to_string(movesRemaining);
    renderer.drawText(movesText, 20, 50);

    renderer.drawText("Click to use a move and earn points", 250, 300);
}

void Game::switchToCity() {
    currentState = GameState::CITY;
    std::cout << "Switched to city" << std::endl;
}

void Game::switchToArena() {
    currentState = GameState::ARENA;
    movesRemaining = 10;
    std::cout << "Switched to arena" << std::endl;
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