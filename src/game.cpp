#include "game.h"
#include <iostream>
#include <fstream>
#include <filesystem>

Game::Game() : currentState(GameState::CITY), isRunning(true), mouseX(0), mouseY(0),
               score(0), movesRemaining(10), playerSelected(false),
               window(nullptr), glContext(nullptr), imguiInitialized(false),
               inCombat(false) {

    uiManagerCity = new UIManager(1024, 768);
    uiManagerArena = new UIManager(1024, 768);

    tileMap = new TileMap(32, 1024, 768);
    mapEditor = new MapEditor(tileMap);

    player = new Player(100, 100);

    combatManager = new CombatManager(player, tileMap);
}

Game::~Game() {
    delete player;
    delete tileMap;
    delete mapEditor;
    delete combatManager;

    delete uiManagerCity;
    delete uiManagerArena;

    for (auto entity : entities) {
        delete entity;
    }

    entities.clear();
}

bool Game::initialize() {
    tileMap->initialize();

    currentCity = "default";
    currentArena = "arena";

    std::filesystem::create_directories("maps");

    bool cityLoaded = loadMap(currentCity);
    if (!cityLoaded) {
        std::cout << "No city map found. Use the editor to create one." << std::endl;

        switchToEditor();
        mapEditor->saveMap("maps/default.json");
        mapEditor->saveMap("maps/arena.json");

        loadMap(currentCity);
    }

    placePlayerInValidPosition();
    return true;
}

bool Game::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Renderer* renderer = SDL_GetRenderer(window);
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    imguiInitialized = true;
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

    if (!renderer.loadTexture("base_limit", "assets/base_limit.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("border_grass", "assets/border_grass.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("border1", "assets/border1.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("border2", "assets/border2.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("border_path", "assets/border_path.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("border_water", "assets/border_water.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("enemy", "assets/enemy.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("player_selection", "assets/player_selection.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("tile_selection", "assets/tile_selection.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("tile_selection_enemy", "assets/tile_selection_enemy.png")){
        std::cout << "Warning: Tile Wall sprite not found. Using placeholder." << std::endl;
    }

    if (!renderer.loadTexture("ui_menu_background", "assets/ui_menu_background.png")){
        std::cout << "Warning: UI Menu Background sprite not found. Using placeholder." << std::endl;
    }

    initializeUI();

    return true;
}

void Game::initializeUI() {
    menuPanel = new UIPanel(0, 10, 992, 98, "ui_menu_background", UIAnchor::BOTTOM_CENTER);

    arenaButton = new UIButton(50, 30, 120, 40, "Go to Arena");
    arenaButton->setOnClick([this]() {
        switchToArena();
    });

    upgradesButton = new UIButton(200, 30, 120, 40, "Upgrades");
    upgradesButton->setOnClick([this]() {
        std::cout << "Upgrades menu clicked" << std::endl;
    });

    statsLabel = new UILabel(350, 30, "Score: 0");

    menuPanel->addElement(arenaButton);
    menuPanel->addElement(upgradesButton);
    menuPanel->addElement(statsLabel);

    uiManagerCity->addElement(menuPanel);

    cityButton = new UIButton(50, 30, 120, 40, "Return to City");
    cityButton->setOnClick([this]() {
        switchToCity();
    });

    UIPanel* arenaMenuPanel = new UIPanel(0, 10, 992, 98, "ui_menu_background", UIAnchor::BOTTOM_CENTER);
    arenaMenuPanel->addElement(cityButton);

    UILabel* waveLabel = new UILabel(200, 30, "Wave: 1/5");
    arenaMenuPanel->addElement(waveLabel);

    uiManagerArena->addElement(arenaMenuPanel);
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

    bool uiHandled = false;
    switch (currentState) {
        case GameState::CITY:
            uiHandled = uiManagerCity->handleEvent(e);
            break;
        case GameState::ARENA:
            uiHandled = uiManagerArena->handleEvent(e);
            break;
        default:
            break;
    }

    if (!uiHandled) {
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
}

void Game::handleCityEvents(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
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
            if ((!player->hasAttacksRemaining() || !inCombat) &&
                mouseX >= cityButton.x && mouseX <= cityButton.x + cityButton.w &&
                mouseY >= cityButton.y && mouseY <= cityButton.y + cityButton.h) {
                switchToCity();
                return;
            }

            int gridX, gridY;
            tileMap->pixelToGrid(mouseX, mouseY, gridX, gridY);

            if (player->isPointOnPlayer(mouseX, mouseY)) {
                player->setSelected(true);
                playerSelected = true;

                player->calculateAvailableTiles(tileMap);
                player->calculateAttackTargets(tileMap);

                std::cout << "Player selected. Attack targets: " << player->getAttackTargets().size() << std::endl;
            }
            else if (playerSelected) {
                int enemyIndex = combatManager->getEnemyAt(gridX, gridY);
                bool inAttackRange = player->isTileInAttackRange(gridX, gridY);

                std::cout << "Clicked grid: " << gridX << "," << gridY << std::endl;
                std::cout << "Enemy at position: " << (enemyIndex != -1 ? "Yes" : "No") << std::endl;
                std::cout << "In attack range: " << (inAttackRange ? "Yes" : "No") << std::endl;

                if (inCombat && enemyIndex != -1 && inAttackRange) {
                    std::cout << "Attacking enemy at " << gridX << "," << gridY << std::endl;
                    combatManager->playerAttack(enemyIndex);
                    player->setSelected(false);
                    playerSelected = false;
                }
                else if (tileMap->isWalkable(gridX, gridY) &&
                         player->isTileAvailable(gridX, gridY)) {
                    int playerGridX, playerGridY;
                    tileMap->pixelToGrid(player->getX(), player->getY(), playerGridX, playerGridY);

                    std::vector<std::pair<int, int>> path =
                        tileMap->findPath(playerGridX, playerGridY, gridX, gridY);

                    if (!path.empty()) {
                        player->setPath(path);
                        player->setSelected(false);
                        playerSelected = false;
                    }
                } else {
                    std::cout << "Invalid move or attack target" << std::endl;
                }
            }

            if (inCombat && !player->hasAttacksRemaining()) {
                std::cout << "Out of attacks! Returning to city..." << std::endl;
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

    if (inCombat) {
        combatManager->update();

        if (!combatManager->isInCombat()) {
            inCombat = false;
            switchToCity();
        }
    }

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
            tileMap->render(renderer);
            renderMovementRange(renderer);
            for (auto entity : entities) {
                entity->render(renderer);
            }
            player->render(renderer);
            uiManagerCity->render(renderer);
            break;
        case GameState::ARENA:
            tileMap->render(renderer);
            renderArena(renderer);
            renderMovementRange(renderer);
            for (auto entity : entities) {
                entity->render(renderer);
            }
            player->render(renderer);
            uiManagerArena->render(renderer);
            break;
        case GameState::EDITOR:
            renderEditor(renderer);
            break;
    }
}

void Game::renderCity(Renderer& renderer) {
}

void Game::renderArena(Renderer& renderer) {
    if (playerSelected) {
        const auto& availableTiles = player->getAvailableTiles();
        for (const auto& tile : availableTiles) {
            int pixelX, pixelY;
            tileMap->gridToPixel(tile.first, tile.second, pixelX, pixelY);

            renderer.renderTexture("tile_selection", pixelX, pixelY,
                                 tileMap->getTileSize(), tileMap->getTileSize());
        }
    }

    if (playerSelected) {
        const auto& attackTargets = player->getAttackTargets();
        for (const auto& tile : attackTargets) {
            int pixelX, pixelY;
            tileMap->gridToPixel(tile.first, tile.second, pixelX, pixelY);

            int enemyIndex = combatManager->getEnemyAt(tile.first, tile.second);
            if (inCombat && enemyIndex != -1) {
                renderer.renderTexture("tile_selection_enemy", pixelX, pixelY,
                                     tileMap->getTileSize(), tileMap->getTileSize());
            } else {
                renderer.renderTexture("tile_selection_enemy", pixelX, pixelY,
                                     tileMap->getTileSize(), tileMap->getTileSize());
            }
        }
    }

    if (!player->hasAttacksRemaining() || !inCombat) {
        renderer.setDrawColor(200, 100, 100, 255);
        renderer.fillRect(cityButton);

        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText("Return to City", cityButton.x + 10, cityButton.y + 10);
    }

    if (inCombat) {
        combatManager->render(renderer);

        std::string waveText = "Wave: " + std::to_string(combatManager->getCurrentWave());
        renderer.drawText(waveText, 10, 10);

        std::string attacksText = "Attacks: " + std::to_string(player->getRemainingAttacks());
        renderer.drawText(attacksText, 10, 30);
    }
}

void Game::renderEditor(Renderer& renderer) {
    renderer.setDrawColor(100, 100, 100, 255);
    renderer.fillRect(0, 0, 1024, 768);

    tileMap->render(renderer);
    mapEditor->render(renderer);
}

void Game::switchToCity() {
    currentState = GameState::CITY;
    mapEditor->setActive(false);
    loadMap(currentCity);
    placePlayerInValidPosition();

    if (inCombat) {
        combatManager->endCombat();
        inCombat = false;
    }

    std::cout << "Switched to city: " << currentCity << std::endl;
}

void Game::switchToArena() {
    currentState = GameState::ARENA;
    mapEditor->setActive(false);
    loadMap(currentArena);
    placePlayerInValidPosition();

    player->setRemainingAttacks(player->getMaxAttacks());

    combatManager->startCombat(1);
    inCombat = true;

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
    shutdownImGui();
    // Cleanup I guesss.
}

void Game::renderMovementRange(Renderer& renderer) {
    if (!playerSelected) return;

    const auto& availableTiles = player->getAvailableTiles();
    for (const auto& tile : availableTiles) {
        int pixelX, pixelY;
        tileMap->gridToPixel(tile.first, tile.second, pixelX, pixelY);

        renderer.renderTexture("tile_selection", pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize());
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

void Game::shutdownImGui() {
    if (imguiInitialized) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        imguiInitialized = false;
    }
}