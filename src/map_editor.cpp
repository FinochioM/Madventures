#include "map_editor.h"
#include <iostream>
#include <fstream>
#include <json.hpp>

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
    editingPropertyValue(false) {

    paletteArea = {600, 20, 180, 200};
    layerButtonsArea = {600, 230, 180, 80};
    toolButtonsArea = {600, 320, 180, 80};
    propertiesArea = {600, 410, 180, 170};

    initializeAvailableTiles();
}

MapEditor::~MapEditor() {
    // Cleanup
}

void MapEditor::initializeAvailableTiles() {
    availableTiles.push_back({"tile_grass", "Grass", true});
    availableTiles.push_back({"tile_wall", "Wall", false});
    //availableTiles.push_back({"tile_water", "Water", false});
    // Add more tiles as needed
}

void MapEditor::handleEvent(SDL_Event& e) {
    if (!active) return;

    if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouseX, &mouseY);
        tileMap->pixelToGrid(mouseX, mouseY, gridX, gridY);
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            handleLeftClick(mouseX, mouseY);
        } else if (e.button.button == SDL_BUTTON_RIGHT) {
            handleRightClick(mouseX, mouseY);
        }
    }

    if (e.type == SDL_KEYDOWN) {
        if (showPropertyPanel && editingPropertyValue) {
            if (e.key.keysym.sym == SDLK_RETURN) {
                editingPropertyValue = false;
            } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                editingPropertyValue = false;
                editingProperty = "";
            }
        } else {
            switch (e.key.keysym.sym) {
                case SDLK_1:
                    currentTool = EditorTool::PENCIL;
                    break;
                case SDLK_2:
                    currentTool = EditorTool::ERASER;
                    break;
                case SDLK_3:
                    currentTool = EditorTool::PROPERTY_EDITOR;
                    break;
                case SDLK_q:
                    currentLayer = EditorLayer::GROUND;
                    break;
                case SDLK_w:
                    currentLayer = EditorLayer::OBJECTS;
                    break;
                case SDLK_e:
                    currentLayer = EditorLayer::COLLISION;
                    break;
                case SDLK_s:
                    if (e.key.keysym.mod & KMOD_CTRL) {
                        saveMap("map.json");
                    }
                    break;
                case SDLK_l:
                    if (e.key.keysym.mod & KMOD_CTRL) {
                        loadMap("map.json");
                    }
                    break;
            }
        }
    }
}

void MapEditor::handleLeftClick(int mouseX, int mouseY) {
    if (isPointInRect(mouseX, mouseY, paletteArea)) {
        int relativeY = mouseY - paletteArea.y;
        int tileIndex = relativeY / 30;

        if (tileIndex >= 0 && tileIndex < availableTiles.size()) {
            currentTileIndex = tileIndex;
        }
        return;
    }

    if (isPointInRect(mouseX, mouseY, layerButtonsArea)) {
        int relativeY = mouseY - layerButtonsArea.y;
        if (relativeY < 30) {
            currentLayer = EditorLayer::GROUND;
        } else if (relativeY < 60) {
            currentLayer = EditorLayer::OBJECTS;
        } else {
            currentLayer = EditorLayer::COLLISION;
        }
        return;
    }

    if (isPointInRect(mouseX, mouseY, toolButtonsArea)) {
        int relativeY = mouseY - toolButtonsArea.y;
        if (relativeY < 30) {
            currentTool = EditorTool::PENCIL;
        } else if (relativeY < 60) {
            currentTool = EditorTool::ERASER;
        } else {
            currentTool = EditorTool::PROPERTY_EDITOR;
        }
        return;
    }

    if (showPropertyPanel && isPointInRect(mouseX, mouseY, propertiesArea)) {
        int relativeY = mouseY - propertiesArea.y;
        int propertyIndex = relativeY / 30;

        if (propertyIndex == 0) {
            editingProperty = "walkable";
            editingPropertyValue = true;
        }
        return;
    }

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

void MapEditor::handleRightClick(int mouseX, int mouseY) {
    if (showPropertyPanel) {
        showPropertyPanel = false;
        editingProperty = "";
        editingPropertyValue = false;
    }
}

void MapEditor::applyTileAtPosition(int gridX, int gridY) {
    if (currentTileIndex >= 0 && currentTileIndex < availableTiles.size()) {
        TileTexture& selectedTile = availableTiles[currentTileIndex];

        Tile* tile = tileMap->getTileAt(gridX, gridY);
        if (tile) {
            switch (currentLayer) {
                case EditorLayer::GROUND:
                    tileMap->setTileTexture(gridX, gridY, selectedTile.id);
                    break;
                case EditorLayer::OBJECTS:
                    tile->setProperty("objectTexture", selectedTile.id);
                    break;
                case EditorLayer::COLLISION:
                    tile->setProperty("walkable", selectedTile.walkable);
                    break;
            }
        }
    }
}

void MapEditor::eraseTileAtPosition(int gridX, int gridY) {
    Tile* tile = tileMap->getTileAt(gridX, gridY);
    if (tile) {
        switch (currentLayer) {
            case EditorLayer::GROUND:
                tileMap->setTileTexture(gridX, gridY, "");
                break;
            case EditorLayer::OBJECTS:
                tile->setProperty("objectTexture", "");
                break;
            case EditorLayer::COLLISION:
                tile->setProperty("walkable", true);
                break;
        }
    }
}

void MapEditor::openPropertyEditor(int gridX, int gridY) {
    Tile* tile = tileMap->getTileAt(gridX, gridY);
    if (tile) {
        showPropertyPanel = true;
        tile->setProperty("_editorX", gridX);
        tile->setProperty("_editorY", gridY);
    }
}

void MapEditor::update() {
    if (!active) return;

    // Continuous painting when mouse button is held down could be implemented here
}

void MapEditor::render(Renderer& renderer) {
    if (!active) return;

    renderer.setDrawColor(80, 80, 80, 100);
    for (int y = 0; y <= tileMap->getGridHeight(); y++) {
        renderer.drawRect(0, y * tileMap->getTileSize(), tileMap->getGridWidth() * tileMap->getTileSize(), 1);
    }
    for (int x = 0; x <= tileMap->getGridWidth(); x++) {
        renderer.drawRect(x * tileMap->getTileSize(), 0, 1, tileMap->getGridHeight() * tileMap->getTileSize());
    }

    if (tileMap->isValidGridPosition(gridX, gridY)) {
        int pixelX, pixelY;
        tileMap->gridToPixel(gridX, gridY, pixelX, pixelY);

        renderer.setDrawColor(255, 255, 0, 100);
        renderer.drawRect(pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize());
    }

    renderPalette(renderer);
    renderLayerButtons(renderer);
    renderToolButtons(renderer);

    if (showPropertyPanel) {
        renderPropertyPanel(renderer);
    }
}

void MapEditor::renderPalette(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(paletteArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Tile Palette", paletteArea.x + 10, paletteArea.y + 5);

    for (size_t i = 0; i < availableTiles.size(); i++) {
        int y = paletteArea.y + 30 + i * 30;

        if (currentTileIndex == i) {
            renderer.setDrawColor(100, 100, 255, 200);
            renderer.fillRect(paletteArea.x + 5, y, paletteArea.w - 10, 25);
        }

        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText(availableTiles[i].name, paletteArea.x + 10, y + 5);
    }
}

void MapEditor::renderLayerButtons(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(layerButtonsArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Layers", layerButtonsArea.x + 10, layerButtonsArea.y + 5);

    if (currentLayer == EditorLayer::GROUND) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(layerButtonsArea.x + 5, layerButtonsArea.y + 30, layerButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Ground (Q)", layerButtonsArea.x + 10, layerButtonsArea.y + 35);

    if (currentLayer == EditorLayer::OBJECTS) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(layerButtonsArea.x + 5, layerButtonsArea.y + 55, layerButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Objects (W)", layerButtonsArea.x + 10, layerButtonsArea.y + 60);

    if (currentLayer == EditorLayer::COLLISION) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(layerButtonsArea.x + 5, layerButtonsArea.y + 80, layerButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Collision (E)", layerButtonsArea.x + 10, layerButtonsArea.y + 85);
}

void MapEditor::renderToolButtons(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(toolButtonsArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Tools", toolButtonsArea.x + 10, toolButtonsArea.y + 5);

    if (currentTool == EditorTool::PENCIL) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(toolButtonsArea.x + 5, toolButtonsArea.y + 30, toolButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Pencil (1)", toolButtonsArea.x + 10, toolButtonsArea.y + 35);

    if (currentTool == EditorTool::ERASER) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(toolButtonsArea.x + 5, toolButtonsArea.y + 55, toolButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Eraser (2)", toolButtonsArea.x + 10, toolButtonsArea.y + 60);

    if (currentTool == EditorTool::PROPERTY_EDITOR) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(toolButtonsArea.x + 5, toolButtonsArea.y + 80, toolButtonsArea.w - 10, 20);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Properties (3)", toolButtonsArea.x + 10, toolButtonsArea.y + 85);
}

void MapEditor::renderPropertyPanel(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(propertiesArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Tile Properties", propertiesArea.x + 10, propertiesArea.y + 5);

    Tile* tile = nullptr;
    if (tileMap->isValidGridPosition(gridX, gridY)) {
        tile = tileMap->getTileAt(gridX, gridY);
    }

    if (tile) {
        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText("Walkable:", propertiesArea.x + 10, propertiesArea.y + 35);

        bool isWalkable = tile->getProperty("walkable", false);
        std::string walkableValue = isWalkable ? "true" : "false";

        if (editingProperty == "walkable" && editingPropertyValue) {
            renderer.setDrawColor(100, 100, 255, 200);
            renderer.fillRect(propertiesArea.x + 100, propertiesArea.y + 30, 70, 20);
        }

        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText(walkableValue, propertiesArea.x + 110, propertiesArea.y + 35);

        renderer.drawText("Texture:", propertiesArea.x + 10, propertiesArea.y + 65);
        std::string textureID = tile->getProperty<std::string>("textureID", "none");
        renderer.drawText(textureID, propertiesArea.x + 110, propertiesArea.y + 65);

        renderer.drawText("Click to edit", propertiesArea.x + 10, propertiesArea.y + 140);
        renderer.drawText("Right-click to close", propertiesArea.x + 10, propertiesArea.y + 160);
    }
}

bool MapEditor::isPointInRect(int x, int y, const SDL_Rect& rect) const {
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
}

bool MapEditor::saveMap(const std::string& filename) {
    try {
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

        // TODO: Reinitialize tilemap with loaded dimensions if they differ

        // Load tiles
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