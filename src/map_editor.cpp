#include "map_editor.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <json.hpp>
#include "imgui/imgui.h"

MapEditor::MapEditor(TileMap* tileMap) :
    tileMap(tileMap),
    active(false),
    mouseX(0),
    mouseY(0),
    gridX(0),
    gridY(0),
    currentTool(EditorTool::PENCIL),
    currentLayer(EditorLayer::GROUND),
    currentTileIndex(0),
    showPropertyPanel(false),
    editingPropertyValue(false),
    currentMapName("default"),
    showMapBrowser(false),
    isNamingMap(false),
    isSelectingMap(false),
    hasTileSelected(false),
    selectedTileX(0),
    selectedTileY(0),
    showDemoWindow(false),
    showAboutWindow(false),
    showMetricsWindow(false),
    gridColor(0.3f, 0.3f, 0.3f, 0.5f),
    selectedTileColor(0.0f, 1.0f, 1.0f, 0.6f),
    cursorColor(1.0f, 1.0f, 0.0f, 0.4f),
    gridOpacity(0.5f),
    isMouseButtonDown(false),
    isCtrlPressed(false) {

    strcpy(inputMapNameBuffer, currentMapName.c_str());

    initializeAvailableTiles();
    refreshMapList();
}

MapEditor::~MapEditor() {
    // Cleanup
}

void MapEditor::initializeAvailableTiles() {
    availableTiles.push_back({"tile_grass", "Grass", true});
    availableTiles.push_back({"tile_wall", "Wall", false});
    availableTiles.push_back({"base_limit", "Limit", false});
    availableTiles.push_back({"border1", "Border 1", false});
    availableTiles.push_back({"border2", "Border 2", false});
    availableTiles.push_back({"border_grass", "Border Grass", false});
    availableTiles.push_back({"border_path", "Border Path", false});
    availableTiles.push_back({"border_water", "Border Water", false});
}

void MapEditor::handleEvent(SDL_Event& e) {
    if (!active) return;

    if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouseX, &mouseY);
        tileMap->pixelToGrid(mouseX, mouseY, gridX, gridY);

        if (isMouseButtonDown && tileMap->isValidGridPosition(gridX, gridY)) {
            switch (currentTool) {
                case EditorTool::PENCIL:
                    applyTileAtPosition(gridX, gridY);
                    break;
                case EditorTool::ERASER:
                    eraseTileAtPosition(gridX, gridY);
                    break;
                case EditorTool::PROPERTY_EDITOR:
                    break;
            }
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                isMouseButtonDown = true;
                if (tileMap->isValidGridPosition(gridX, gridY)) {
                    switch (currentTool) {
                        case EditorTool::PENCIL:
                            applyTileAtPosition(gridX, gridY);
                            break;
                        case EditorTool::ERASER:
                            eraseTileAtPosition(gridX, gridY);
                            break;
                        case EditorTool::PROPERTY_EDITOR:
                            openPropertyEditor(gridX, gridY);
                            break;
                    }
                }
            }
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            isMouseButtonDown = false;
        }
    }

    if (e.type == SDL_KEYDOWN) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                isCtrlPressed = true;
            }
            else if (isCtrlPressed) {
                switch (e.key.keysym.sym) {
                    case SDLK_z:
                        undo();
                        break;
                    case SDLK_y:
                        redo();
                        break;
                    case SDLK_a:
                        selectAll();
                        break;
                }
            }
            else if (e.key.keysym.sym == SDLK_ESCAPE) {
                hasTileSelected = false;
            }
        }
    }

    if (e.type == SDL_KEYUP) {
        if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
            isCtrlPressed = false;
        }
    }
}

void MapEditor::update() {
    if (!active) return;
}

void MapEditor::render(Renderer& renderer) {
    if (!active) return;

    renderer.setDrawColor(
        static_cast<Uint8>(gridColor.x * 255),
        static_cast<Uint8>(gridColor.y * 255),
        static_cast<Uint8>(gridColor.z * 255),
        static_cast<Uint8>(gridColor.w * 255 * gridOpacity));

    for (int y = 0; y <= tileMap->getGridHeight(); y++) {
        renderer.drawRect(0, y * tileMap->getTileSize(), tileMap->getGridWidth() * tileMap->getTileSize(), 1);
    }
    for (int x = 0; x <= tileMap->getGridWidth(); x++) {
        renderer.drawRect(x * tileMap->getTileSize(), 0, 1, tileMap->getGridHeight() * tileMap->getTileSize());
    }

    if (tileMap->isValidGridPosition(gridX, gridY)) {
        int pixelX, pixelY;
        tileMap->gridToPixel(gridX, gridY, pixelX, pixelY);

        renderer.setDrawColor(
            static_cast<Uint8>(cursorColor.x * 255),
            static_cast<Uint8>(cursorColor.y * 255),
            static_cast<Uint8>(cursorColor.z * 255),
            static_cast<Uint8>(cursorColor.w * 255));
        renderer.drawRect(pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize());
    }

    if (hasTileSelected) {
        int pixelX, pixelY;
        tileMap->gridToPixel(selectedTileX, selectedTileY, pixelX, pixelY);

        renderer.setDrawColor(
            static_cast<Uint8>(selectedTileColor.x * 255),
            static_cast<Uint8>(selectedTileColor.y * 255),
            static_cast<Uint8>(selectedTileColor.z * 255),
            static_cast<Uint8>(selectedTileColor.w * 255));
        renderer.drawRect(pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize());
    }

    renderImGuiInterface();
}

void MapEditor::renderImGuiInterface() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Map")) {
                // Handle new map creation
            }
            if (ImGui::MenuItem("Open Map")) {
                showMapBrowser = true;
                isSelectingMap = true;
                isNamingMap = false;
                refreshMapList();
            }
            if (ImGui::MenuItem("Save Map")) {
                saveMap(getMapPath(currentMapName));
            }
            if (ImGui::MenuItem("Save Map As...")) {
                showMapBrowser = true;
                isNamingMap = true;
                isSelectingMap = false;
                strcpy(inputMapNameBuffer, currentMapName.c_str());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit Editor")) {
                setActive(false);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !undoStack.empty())) {
                undo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !redoStack.empty())) {
                redo();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {
                selectAll();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Properties Panel", NULL, &showPropertyPanel);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", NULL, &showDemoWindow);
            ImGui::MenuItem("ImGui Metrics", NULL, &showMetricsWindow);
            ImGui::MenuItem("About ImGui", NULL, &showAboutWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    renderToolsPanel();

    renderLayersPanel();

    renderTilePalette();

    if (showPropertyPanel) {
        renderPropertiesPanel();
    }

    if (showMapBrowser) {
        renderMapBrowser();
    }

    renderStatusBar();

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    if (showMetricsWindow) {
        ImGui::ShowMetricsWindow(&showMetricsWindow);
    }

    if (showAboutWindow) {
        ImGui::ShowAboutWindow(&showAboutWindow);
    }
}

void MapEditor::renderToolsPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(180, 120), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Tools", nullptr)) {
        const char* toolNames[] = { "Pencil", "Eraser", "Property Editor" };
        int toolIndex = static_cast<int>(currentTool);

        if (ImGui::Combo("Tool", &toolIndex, toolNames, IM_ARRAYSIZE(toolNames))) {
            currentTool = static_cast<EditorTool>(toolIndex);
        }

        ImGui::Separator();

        if (ImGui::Button("Pencil (1)", ImVec2(150, 0))) {
            currentTool = EditorTool::PENCIL;
        }

        if (ImGui::Button("Eraser (2)", ImVec2(150, 0))) {
            currentTool = EditorTool::ERASER;
        }

        if (ImGui::Button("Property Editor (3)", ImVec2(150, 0))) {
            currentTool = EditorTool::PROPERTY_EDITOR;
        }
    }
    ImGui::End();
}

void MapEditor::renderLayersPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 160), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(180, 120), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Layers", nullptr)) {
        const char* layerNames[] = { "Ground", "Objects", "Collision" };
        int layerIndex = static_cast<int>(currentLayer);

        if (ImGui::Combo("Layer", &layerIndex, layerNames, IM_ARRAYSIZE(layerNames))) {
            currentLayer = static_cast<EditorLayer>(layerIndex);
        }

        ImGui::Separator();

        if (ImGui::Button("Ground (Q)", ImVec2(150, 0))) {
            currentLayer = EditorLayer::GROUND;
        }

        if (ImGui::Button("Objects (W)", ImVec2(150, 0))) {
            currentLayer = EditorLayer::OBJECTS;
        }

        if (ImGui::Button("Collision (E)", ImVec2(150, 0))) {
            currentLayer = EditorLayer::COLLISION;
        }
    }
    ImGui::End();
}

void MapEditor::renderTilePalette() {
    ImGui::SetNextWindowPos(ImVec2(10, 290), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(180, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Tile Palette", nullptr)) {
        ImGui::Text("Available Tiles");
        ImGui::Separator();

        for (size_t i = 0; i < availableTiles.size(); i++) {
            bool isSelected = (currentTileIndex == i);
            if (ImGui::Selectable(availableTiles[i].name.c_str(), isSelected)) {
                currentTileIndex = i;
            }

            if (isSelected && ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Texture ID: %s", availableTiles[i].id.c_str());
                ImGui::Text("Walkable: %s", availableTiles[i].walkable ? "Yes" : "No");
                ImGui::EndTooltip();
            }
        }
    }
    ImGui::End();
}

void MapEditor::renderPropertiesPanel() {
    ImGui::SetNextWindowPos(ImVec2(600, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(180, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Properties", &showPropertyPanel)) {
        if (hasTileSelected && tileMap->isValidGridPosition(selectedTileX, selectedTileY)) {
            Tile* tile = tileMap->getTileAt(selectedTileX, selectedTileY);
            if (tile) {
                ImGui::Text("Selected Tile: %d,%d", selectedTileX, selectedTileY);
                ImGui::Separator();

                bool isWalkable = tile->getProperty("walkable", true);
                if (ImGui::Checkbox("Walkable", &isWalkable)) {
                    tile->setProperty("walkable", isWalkable);
                }

                std::string textureID = tile->getProperty<std::string>("textureID", "");
                ImGui::Text("Texture ID: %s", textureID.c_str());

                std::string objectTexture = tile->getProperty<std::string>("objectTexture", "");
                ImGui::Text("Object Texture: %s", objectTexture.c_str());
            }
        } else {
            ImGui::Text("No tile selected");
            ImGui::Text("Use the Property Tool (3)");
            ImGui::Text("to select a tile");
        }
    }
    ImGui::End();
}

void MapEditor::renderMapBrowser() {
    ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);

    const char* title = isNamingMap ? "Save Map As" : "Open Map";

    ImGui::Begin(title, &showMapBrowser, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (isNamingMap) {
        ImGui::Text("Enter Map Name:");

        if (ImGui::InputText("##MapName", inputMapNameBuffer, sizeof(inputMapNameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(inputMapNameBuffer) > 0) {
                currentMapName = inputMapNameBuffer;
                saveMap(getMapPath(currentMapName));
                showMapBrowser = false;
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            if (strlen(inputMapNameBuffer) > 0) {
                currentMapName = inputMapNameBuffer;
                saveMap(getMapPath(currentMapName));
                showMapBrowser = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            showMapBrowser = false;
        }
    } else if (isSelectingMap) {
        ImGui::Text("Select a Map:");
        ImGui::Separator();

        for (const auto& mapName : availableMaps) {
            if (ImGui::Selectable(mapName.c_str(), currentMapName == mapName)) {
                currentMapName = mapName;
                loadMap(getMapPath(currentMapName));
                showMapBrowser = false;
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            showMapBrowser = false;
        }
    }

    ImGui::End();
}

void MapEditor::renderStatusBar() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20));
    ImGui::Begin("##StatusBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::Text("Position: %d,%d", gridX, gridY);

    ImGui::SameLine(150);

    ImGui::Text("Map: %s", currentMapName.c_str());

    ImGui::SameLine(300);

    const char* toolNames[] = { "Pencil", "Eraser", "Property Editor" };
    const char* layerNames[] = { "Ground", "Objects", "Collision" };

    ImGui::Text("Tool: %s | Layer: %s",
                toolNames[static_cast<int>(currentTool)],
                layerNames[static_cast<int>(currentLayer)]);

    ImGui::SameLine(ImGui::GetWindowWidth() - 100);

    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Map Editor Controls:");
        ImGui::Text("1-3: Change tools");
        ImGui::Text("Q-E: Change layers");
        ImGui::Text("Ctrl+S: Save map");
        ImGui::Text("Ctrl+L: Load map");
        ImGui::EndTooltip();
    }

    ImGui::End();
}

void MapEditor::applyTileAtPosition(int gridX, int gridY) {
    if (currentTileIndex >= 0 && currentTileIndex < availableTiles.size()) {
        TileTexture& selectedTile = availableTiles[currentTileIndex];

        Tile* tile = tileMap->getTileAt(gridX, gridY);
        if (tile) {
            TileAction action;
            action.gridX = gridX;
            action.gridY = gridY;
            action.layer = currentLayer;

            switch (currentLayer) {
                case EditorLayer::GROUND:
                    action.oldTextureID = tile->getProperty<std::string>("textureID", "");
                    action.newTextureID = selectedTile.id;
                    if (action.oldTextureID == action.newTextureID) {
                        return;
                    }
                    tileMap->setTileTexture(gridX, gridY, selectedTile.id);
                    break;
                case EditorLayer::OBJECTS:
                    action.oldObjectTexture = tile->getProperty<std::string>("objectTexture", "");
                    action.newObjectTexture = selectedTile.id;
                    if (action.oldObjectTexture == action.newObjectTexture) {
                        return;
                    }
                    tile->setProperty("objectTexture", selectedTile.id);
                    break;
                case EditorLayer::COLLISION:
                    action.oldWalkable = tile->getProperty("walkable", true);
                    action.newWalkable = selectedTile.walkable;
                    if (action.oldWalkable == action.newWalkable) {
                        return;
                    }
                    tile->setProperty("walkable", selectedTile.walkable);
                    break;
            }

            recordAction(action);
        }
    }
}

void MapEditor::eraseTileAtPosition(int gridX, int gridY) {
    Tile* tile = tileMap->getTileAt(gridX, gridY);
    if (tile) {
        TileAction action;
        action.gridX = gridX;
        action.gridY = gridY;
        action.layer = currentLayer;

        switch (currentLayer) {
            case EditorLayer::GROUND:
                action.oldTextureID = tile->getProperty<std::string>("textureID", "");
                action.newTextureID = "";
                if (action.oldTextureID.empty()) {
                    return;
                }
                tileMap->setTileTexture(gridX, gridY, "");
                break;
            case EditorLayer::OBJECTS:
                action.oldObjectTexture = tile->getProperty<std::string>("objectTexture", "");
                action.newObjectTexture = "";
                if (action.oldObjectTexture.empty()) {
                    return;
                }
                tile->setProperty("objectTexture", "");
                break;
            case EditorLayer::COLLISION:
                action.oldWalkable = tile->getProperty("walkable", false);
                action.newWalkable = true;
                if (action.oldWalkable == true) {
                    return;
                }
                tile->setProperty("walkable", true);
                break;
        }

        recordAction(action);
    }
}

void MapEditor::openPropertyEditor(int gridX, int gridY) {
    Tile* tile = tileMap->getTileAt(gridX, gridY);
    if (tile) {
        selectedTileX = gridX;
        selectedTileY = gridY;
        hasTileSelected = true;
        showPropertyPanel = true;
    }
}

bool MapEditor::saveMap(const std::string& filename) {
    try {
        std::filesystem::create_directories("maps");

        nlohmann::json mapJson;

        mapJson["width"] = tileMap->getGridWidth();
        mapJson["height"] = tileMap->getGridHeight();
        mapJson["tileSize"] = tileMap->getTileSize();

        nlohmann::json tilesJson = nlohmann::json::array();

        for (int y = 0; y < tileMap->getGridHeight(); y++) {
            for (int x = 0; x < tileMap->getGridWidth(); x++) {
                Tile* tile = tileMap->getTileAt(x, y);
                if (tile) {
                    nlohmann::json tileJson;
                    tileJson["x"] = x;
                    tileJson["y"] = y;
                    tileJson["walkable"] = tile->getProperty("walkable", false);
                    tileJson["textureID"] = tile->getProperty<std::string>("textureID", "");
                    tileJson["objectTexture"] = tile->getProperty<std::string>("objectTexture", "");

                    tilesJson.push_back(tileJson);
                }
            }
        }

        mapJson["tiles"] = tilesJson;

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        file << mapJson.dump(4);
        file.close();

        std::cout << "Map saved to " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving map: " << e.what() << std::endl;
        return false;
    }
}

bool MapEditor::loadMap(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filename << std::endl;
            return false;
        }

        nlohmann::json mapJson;
        file >> mapJson;
        file.close();

        if (!mapJson.contains("width") || !mapJson.contains("height") ||
            !mapJson.contains("tileSize") || !mapJson.contains("tiles")) {
            std::cerr << "Invalid map file format" << std::endl;
            return false;
        }

        for (int y = 0; y < tileMap->getGridHeight(); y++) {
            for (int x = 0; x < tileMap->getGridWidth(); x++) {
                Tile* tile = tileMap->getTileAt(x, y);
                if (tile) {
                    tile->setProperty("walkable", true);
                    tile->setProperty("textureID", "");
                    tile->setProperty("objectTexture", "");
                }
            }
        }

        for (const auto& tileJson : mapJson["tiles"]) {
            int x = tileJson["x"];
            int y = tileJson["y"];

            if (tileMap->isValidGridPosition(x, y)) {
                Tile* tile = tileMap->getTileAt(x, y);
                if (tile) {
                    if (tileJson.contains("walkable")) {
                        tile->setProperty("walkable", tileJson["walkable"].get<bool>());
                    }

                    if (tileJson.contains("textureID")) {
                        std::string textureID = tileJson["textureID"].get<std::string>();
                        tileMap->setTileTexture(x, y, textureID);
                    }

                    if (tileJson.contains("objectTexture")) {
                        std::string objectTexture = tileJson["objectTexture"].get<std::string>();
                        tile->setProperty("objectTexture", objectTexture);
                    }
                }
            }
        }

        std::cout << "Map loaded from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading map: " << e.what() << std::endl;
        return false;
    }
}

void MapEditor::refreshMapList() {
    scanMapDirectory();
}

std::string MapEditor::getMapPath(const std::string& mapName) {
    return "maps/" + mapName + ".json";
}

void MapEditor::scanMapDirectory() {
    availableMaps.clear();

    try {
        std::filesystem::create_directories("maps");

        for (const auto& entry : std::filesystem::directory_iterator("maps")) {
            if (entry.path().extension() == ".json") {
                std::string filename = entry.path().stem().string();
                availableMaps.push_back(filename);
            }
        }

        if (availableMaps.empty()) {
            availableMaps.push_back("default");
            availableMaps.push_back("city");
            availableMaps.push_back("arena");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning maps directory: " << e.what() << std::endl;
        availableMaps.push_back("default");
        availableMaps.push_back("city");
        availableMaps.push_back("arena");
    }
}

void MapEditor::recordAction(const TileAction& action) {
    undoStack.push_back(action);
    redoStack.clear();
}

void MapEditor::undo() {
    if (undoStack.empty()) return;

    TileAction action = undoStack.back();
    undoStack.pop_back();

    TileAction redoAction = action;
    std::swap(redoAction.oldTextureID, redoAction.newTextureID);
    std::swap(redoAction.oldWalkable, redoAction.newWalkable);
    std::swap(redoAction.oldObjectTexture, redoAction.newObjectTexture);
    redoStack.push_back(redoAction);

    Tile* tile = tileMap->getTileAt(action.gridX, action.gridY);
    if (tile) {
        switch (action.layer) {
            case EditorLayer::GROUND:
                tileMap->setTileTexture(action.gridX, action.gridY, action.oldTextureID);
                break;
            case EditorLayer::OBJECTS:
                tile->setProperty("objectTexture", action.oldObjectTexture);
                break;
            case EditorLayer::COLLISION:
                tile->setProperty("walkable", action.oldWalkable);
                break;
        }
    }
}

void MapEditor::redo() {
    if (redoStack.empty()) return;

    TileAction action = redoStack.back();
    redoStack.pop_back();

    TileAction undoAction = action;
    std::swap(undoAction.oldTextureID, undoAction.newTextureID);
    std::swap(undoAction.oldWalkable, undoAction.newWalkable);
    std::swap(undoAction.oldObjectTexture, undoAction.newObjectTexture);
    undoStack.push_back(undoAction);

    Tile* tile = tileMap->getTileAt(action.gridX, action.gridY);
    if (tile) {
        switch (action.layer) {
            case EditorLayer::GROUND:
                tileMap->setTileTexture(action.gridX, action.gridY, action.newTextureID);
                break;
            case EditorLayer::OBJECTS:
                tile->setProperty("objectTexture", action.newObjectTexture);
                break;
            case EditorLayer::COLLISION:
                tile->setProperty("walkable", action.newWalkable);
                break;
        }
    }
}

void MapEditor::selectAll() {
    if (tileMap->getGridWidth() > 0 && tileMap->getGridHeight() > 0) {
        selectedTileX = 0;
        selectedTileY = 0;
        hasTileSelected = true;
        showPropertyPanel = true;
    }
}
