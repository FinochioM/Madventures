#include "game.h"
#include <iostream>


Game::Game() : currentState(GameState::CITY), isRunning(true), mouseX(0), mouseY(0), score(0), movesRemaining(10), playerSelected(false) {
    arenaButton = {350, 400, 100, 50};

    player = new Player(400, 300);
}

Game::~Game() {
    delete player;

    for (auto entity : entities) {
        delete entity;
    }

    entities.clear();
}

bool Game::initialize() {
    return true;
}

bool Game::loadAssets(Renderer& renderer) {
    if (!renderer.loadTexture("player", "assets/player.png")) {
        std::cout << "Warning: Player sprite not found. Using placeholder." << std::endl;
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
            }else if (playerSelected) {
                player->setTargetPosition(mouseX, mouseY);
                player->setSelected(false);
                playerSelected = false;
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