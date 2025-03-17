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
    editingPropertyValue(false),
    currentMapName("default"),
    showMapBrowser(false),
    isNamingMap(false),
    isSelectingMap(false),
    inputMapName(""),
    palettePanelExpanded(false),
    layersPanelExpanded(false),
    toolsPanelExpanded(false),
    propertiesPanelExpanded(false),
    hasTileSelected(false),
    selectedTileX(0),
    selectedTileY(0) {

    int headerWidth = 150;
    int headerHeight = 30;
    int headerY = 10;

    palettePanelHeaderArea = {20, headerY, headerWidth, headerHeight};
    layersPanelHeaderArea = {20 + headerWidth + 10, headerY, headerWidth, headerHeight};
    toolsPanelHeaderArea = {20 + (headerWidth + 10) * 2, headerY, headerWidth, headerHeight};
    propertiesPanelHeaderArea = {20 + (headerWidth + 10) * 3, headerY, headerWidth, headerHeight};

    int panelWidth = 180;
    int panelHeight = 200;
    int panelY = headerY + headerHeight + 5;

    paletteArea = {palettePanelHeaderArea.x, panelY, panelWidth, panelHeight};
    layerButtonsArea = {layersPanelHeaderArea.x, panelY, panelWidth, panelHeight};
    toolButtonsArea = {toolsPanelHeaderArea.x, panelY, panelWidth, panelHeight};
    propertiesArea = {propertiesPanelHeaderArea.x, panelY, panelWidth, panelHeight};

    saveButtonArea = {20, 560, 100, 30};
    loadButtonArea = {130, 560, 100, 30};
    inputArea = {250, 250, 300, 50};
    mapListArea = {250, 310, 300, 200};

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
    // Add more tiles as needed
}

void MapEditor::handleEvent(SDL_Event& e) {
    if (!active) return;

    if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouseX, &mouseY);
        tileMap->pixelToGrid(mouseX, mouseY, gridX, gridY);
    }

    if (showMapBrowser) {
        if (e.type == SDL_KEYDOWN) {
            if (isNamingMap) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    if (!inputMapName.empty()) {
                        currentMapName = inputMapName;
                        saveMap(getMapPath(currentMapName));
                        showMapBrowser = false;
                        isNamingMap = false;
                    }
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    showMapBrowser = false;
                    isNamingMap = false;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (!inputMapName.empty()) {
                        inputMapName.pop_back();
                    }
                } else if (e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z ||
                           e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9 ||
                           e.key.keysym.sym == SDLK_UNDERSCORE) {
                    char c = (e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z) ?
                            'a' + (e.key.keysym.sym - SDLK_a) :
                            (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) ?
                            '0' + (e.key.keysym.sym - SDLK_0) : '_';

                    if (e.key.keysym.mod & KMOD_SHIFT && c >= 'a' && c <= 'z') {
                        c = 'A' + (c - 'a');
                    }

                    inputMapName += c;
                }
            }
            else if (isSelectingMap) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    showMapBrowser = false;
                    isSelectingMap = false;
                }
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (isSelectingMap) {
                int relativeY = mouseY - mapListArea.y;
                int mapIndex = relativeY / 30;

                if (mouseX >= mapListArea.x && mouseX < mapListArea.x + mapListArea.w &&
                    relativeY >= 0 && mapIndex < availableMaps.size()) {
                    currentMapName = availableMaps[mapIndex];
                    loadMap(getMapPath(currentMapName));
                    showMapBrowser = false;
                    isSelectingMap = false;
                }
            }
        }

        return;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            if (isPointInRect(mouseX, mouseY, palettePanelHeaderArea)) {
                bool wasExpanded = palettePanelExpanded;
                palettePanelExpanded = false;
                layersPanelExpanded = false;
                toolsPanelExpanded = false;
                propertiesPanelExpanded = false;
                if (!wasExpanded) {
                    palettePanelExpanded = true;
                }
                return;
            }

            if (isPointInRect(mouseX, mouseY, layersPanelHeaderArea)) {
                bool wasExpanded = layersPanelExpanded;
                palettePanelExpanded = false;
                layersPanelExpanded = false;
                toolsPanelExpanded = false;
                propertiesPanelExpanded = false;
                if (!wasExpanded) {
                    layersPanelExpanded = true;
                }
                return;
            }

            if (isPointInRect(mouseX, mouseY, toolsPanelHeaderArea)) {
                bool wasExpanded = toolsPanelExpanded;

                palettePanelExpanded = false;
                layersPanelExpanded = false;
                toolsPanelExpanded = false;
                propertiesPanelExpanded = false;
                if (!wasExpanded) {
                    toolsPanelExpanded = true;
                }
                return;
            }

            if (isPointInRect(mouseX, mouseY, propertiesPanelHeaderArea)) {
                bool wasExpanded = propertiesPanelExpanded;
                palettePanelExpanded = false;
                layersPanelExpanded = false;
                toolsPanelExpanded = false;
                propertiesPanelExpanded = false;
                if (!wasExpanded) {
                    propertiesPanelExpanded = true;
                }
                return;
            }

            if (isPointInRect(mouseX, mouseY, saveButtonArea)) {
                showMapBrowser = true;
                isNamingMap = true;
                isSelectingMap = false;
                inputMapName = currentMapName;
                return;
            }

            if (isPointInRect(mouseX, mouseY, loadButtonArea)) {
                showMapBrowser = true;
                isNamingMap = false;
                isSelectingMap = true;
                refreshMapList();
                return;
            }

            if (isPointOverAnyPanel(mouseX, mouseY)) {
                if (palettePanelExpanded && isPointInRect(mouseX, mouseY, paletteArea)) {
                    int relativeY = mouseY - paletteArea.y;
                    int tileIndex = relativeY / 30;

                    if (tileIndex >= 0 && tileIndex < availableTiles.size()) {
                        currentTileIndex = tileIndex;
                    }
                }
                else if (layersPanelExpanded && isPointInRect(mouseX, mouseY, layerButtonsArea)) {
                    int relativeY = mouseY - layerButtonsArea.y;
                    if (relativeY < 30) {
                        currentLayer = EditorLayer::GROUND;
                    } else if (relativeY < 60) {
                        currentLayer = EditorLayer::OBJECTS;
                    } else {
                        currentLayer = EditorLayer::COLLISION;
                    }
                }
                else if (toolsPanelExpanded && isPointInRect(mouseX, mouseY, toolButtonsArea)) {
                    int relativeY = mouseY - toolButtonsArea.y;
                    if (relativeY < 30) {
                        currentTool = EditorTool::PENCIL;
                    } else if (relativeY < 60) {
                        currentTool = EditorTool::ERASER;
                    } else {
                        currentTool = EditorTool::PROPERTY_EDITOR;
                    }
                }
                else if (propertiesPanelExpanded && isPointInRect(mouseX, mouseY, propertiesArea)) {
                    int relativeY = mouseY - propertiesArea.y;
                    int propertyIndex = (relativeY - 30) / 30;

                    if (propertyIndex == 0 && hasTileSelected) {
                        Tile* tile = tileMap->getTileAt(selectedTileX, selectedTileY);
                        if (tile) {
                            bool isWalkable = tile->getProperty("walkable", true);
                            tile->setProperty("walkable", !isWalkable);
                        }
                    }
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
        else if (e.button.button == SDL_BUTTON_RIGHT) {
            if (propertiesPanelExpanded) {
                propertiesPanelExpanded = false;
                hasTileSelected = false;
            }
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
                        if (currentMapName.empty()) {
                            showMapBrowser = true;
                            isNamingMap = true;
                            isSelectingMap = false;
                            inputMapName = "default";
                        } else {
                            saveMap(getMapPath(currentMapName));
                        }
                    }
                    break;
                case SDLK_l:
                    if (e.key.keysym.mod & KMOD_CTRL) {
                        showMapBrowser = true;
                        isNamingMap = false;
                        isSelectingMap = true;
                        refreshMapList();
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
        selectedTileX = gridX;
        selectedTileY = gridY;
        hasTileSelected = true;
        propertiesPanelExpanded = true;
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

    if (hasTileSelected && propertiesPanelExpanded) {
        int pixelX, pixelY;
        tileMap->gridToPixel(selectedTileX, selectedTileY, pixelX, pixelY);

        renderer.setDrawColor(0, 255, 255, 150);
        renderer.drawRect(pixelX, pixelY, tileMap->getTileSize(), tileMap->getTileSize());
    }

    renderer.setDrawColor(80, 80, 180, 255);
    renderer.fillRect(saveButtonArea);
    renderer.setDrawColor(80, 180, 80, 255);
    renderer.fillRect(loadButtonArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Save Map", saveButtonArea.x + 10, saveButtonArea.y + 8);
    renderer.drawText("Load Map", loadButtonArea.x + 10, loadButtonArea.y + 8);

    std::string mapInfo = "Current Map: " + currentMapName;
    renderer.drawText(mapInfo, 20, 530);

    renderer.setDrawColor(40, 40, 80, 200);
    renderer.fillRect(palettePanelHeaderArea);
    renderer.fillRect(layersPanelHeaderArea);
    renderer.fillRect(toolsPanelHeaderArea);
    renderer.fillRect(propertiesPanelHeaderArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Tiles", palettePanelHeaderArea.x + 10, palettePanelHeaderArea.y + 8);
    renderer.drawText("Layers", layersPanelHeaderArea.x + 10, layersPanelHeaderArea.y + 8);
    renderer.drawText("Tools", toolsPanelHeaderArea.x + 10, toolsPanelHeaderArea.y + 8);
    renderer.drawText("Properties", propertiesPanelHeaderArea.x + 10, propertiesPanelHeaderArea.y + 8);

    if (palettePanelExpanded) {
        renderPalette(renderer);
    }

    if (layersPanelExpanded) {
        renderLayerButtons(renderer);
    }

    if (toolsPanelExpanded) {
        renderToolButtons(renderer);
    }

    if (propertiesPanelExpanded) {
        renderPropertyPanel(renderer);
    }

    if (showMapBrowser) {
        renderMapBrowser(renderer);
    }

     //drawUIBounds(renderer);
}

void MapEditor::renderPalette(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(paletteArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Available Tiles", paletteArea.x + 10, paletteArea.y + 5);

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
    renderer.drawText("Select Layer", layerButtonsArea.x + 10, layerButtonsArea.y + 5);

    SDL_Rect groundRect = {layerButtonsArea.x + 5, layerButtonsArea.y + 30, layerButtonsArea.w - 10, 25};
    if (currentLayer == EditorLayer::GROUND) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(groundRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Ground (Q)", groundRect.x + 10, groundRect.y + 5);

    SDL_Rect objectsRect = {layerButtonsArea.x + 5, layerButtonsArea.y + 60, layerButtonsArea.w - 10, 25};
    if (currentLayer == EditorLayer::OBJECTS) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(objectsRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Objects (W)", objectsRect.x + 10, objectsRect.y + 5);

    SDL_Rect collisionRect = {layerButtonsArea.x + 5, layerButtonsArea.y + 90, layerButtonsArea.w - 10, 25};
    if (currentLayer == EditorLayer::COLLISION) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(collisionRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Collision (E)", collisionRect.x + 10, collisionRect.y + 5);
}

void MapEditor::renderToolButtons(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(toolButtonsArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Select Tool", toolButtonsArea.x + 10, toolButtonsArea.y + 5);

    SDL_Rect pencilRect = {toolButtonsArea.x + 5, toolButtonsArea.y + 30, toolButtonsArea.w - 10, 25};
    if (currentTool == EditorTool::PENCIL) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(pencilRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Pencil (1)", pencilRect.x + 10, pencilRect.y + 5);

    SDL_Rect eraserRect = {toolButtonsArea.x + 5, toolButtonsArea.y + 60, toolButtonsArea.w - 10, 25};
    if (currentTool == EditorTool::ERASER) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(eraserRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Eraser (2)", eraserRect.x + 10, eraserRect.y + 5);

    SDL_Rect propRect = {toolButtonsArea.x + 5, toolButtonsArea.y + 90, toolButtonsArea.w - 10, 25};
    if (currentTool == EditorTool::PROPERTY_EDITOR) {
        renderer.setDrawColor(100, 100, 255, 200);
        renderer.fillRect(propRect);
    }
    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Properties (3)", propRect.x + 10, propRect.y + 5);
}

void MapEditor::renderPropertyPanel(Renderer& renderer) {
    renderer.setDrawColor(50, 50, 50, 200);
    renderer.fillRect(propertiesArea);

    renderer.setDrawColor(255, 255, 255, 255);
    renderer.drawText("Tile Properties", propertiesArea.x + 10, propertiesArea.y + 5);

    Tile* tile = nullptr;
    if (hasTileSelected && tileMap->isValidGridPosition(selectedTileX, selectedTileY)) {
        tile = tileMap->getTileAt(selectedTileX, selectedTileY);
    }

    if (tile) {
        renderer.setDrawColor(255, 255, 255, 255);

        std::string posText = "Selected: " + std::to_string(selectedTileX) + "," + std::to_string(selectedTileY);
        renderer.drawText(posText, propertiesArea.x + 10, propertiesArea.y + 35);

        renderer.drawText("Walkable:", propertiesArea.x + 10, propertiesArea.y + 65);

        bool isWalkable = tile->getProperty("walkable", true);

        SDL_Rect toggleRect = {
            propertiesArea.x + 100,
            propertiesArea.y + 62,
            40,
            20
        };

        if (isWalkable) {
            renderer.setDrawColor(0, 200, 0, 255);
        } else {
            renderer.setDrawColor(200, 0, 0, 255);
        }

        renderer.fillRect(toggleRect);

        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText(isWalkable ? "True" : "False", toggleRect.x + 5, toggleRect.y + 3);

        renderer.drawText("Texture:", propertiesArea.x + 10, propertiesArea.y + 95);
        std::string textureID = tile->getProperty<std::string>("textureID", "none");
        renderer.drawText(textureID, propertiesArea.x + 80, propertiesArea.y + 95);

        renderer.drawText("Click to toggle walkable", propertiesArea.x + 10, propertiesArea.y + 140);
        renderer.drawText("Right-click to close panel", propertiesArea.x + 10, propertiesArea.y + 160);
    } else {
        renderer.drawText("No tile selected", propertiesArea.x + 10, propertiesArea.y + 80);
        renderer.drawText("Use the Property Tool", propertiesArea.x + 10, propertiesArea.y + 110);
        renderer.drawText("to select a tile", propertiesArea.x + 10, propertiesArea.y + 130);
    }
}

void MapEditor::renderMapBrowser(Renderer& renderer) {
    renderer.setDrawColor(0, 0, 0, 200);
    renderer.fillRect(0, 0, 800, 600);

    renderer.setDrawColor(50, 50, 50, 255);
    renderer.fillRect(200, 200, 400, 320);

    renderer.setDrawColor(255, 255, 255, 255);

    if (isNamingMap) {
        renderer.drawText("Enter Map Name:", 250, 220);

        renderer.setDrawColor(20, 20, 20, 255);
        renderer.fillRect(inputArea);

        renderer.setDrawColor(255, 255, 255, 255);
        renderer.drawText(inputMapName + "_", inputArea.x + 10, inputArea.y + 15);

        renderer.drawText("Press Enter to save, Escape to cancel", 250, 480);
    }
    else if (isSelectingMap) {
        renderer.drawText("Select a Map:", 250, 220);

        renderer.setDrawColor(20, 20, 20, 255);
        renderer.fillRect(mapListArea);

        renderer.setDrawColor(255, 255, 255, 255);

        int y = mapListArea.y + 10;
        for (const auto& mapName : availableMaps) {
            renderer.drawText(mapName, mapListArea.x + 10, y);
            y += 30;
        }

        renderer.drawText("Click on a map to load, Escape to cancel", 250, 480);
    }
}

bool MapEditor::isPointInRect(int x, int y, const SDL_Rect& rect) const {
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
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

        // TODO: Reinitialize tilemap with loaded dimensions if they differ

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

void MapEditor::drawUIBounds(Renderer& renderer) {
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer.getRenderer(), &r, &g, &b, &a);

    renderer.setDrawColor(255, 0, 0, 255);
    renderer.drawRect(paletteArea);

    renderer.setDrawColor(0, 255, 0, 255);
    renderer.drawRect(layerButtonsArea);

    renderer.setDrawColor(0, 0, 255, 255);
    renderer.drawRect(toolButtonsArea);

    renderer.setDrawColor(255, 255, 0, 255);
    renderer.drawRect(propertiesArea);

    renderer.setDrawColor(255, 0, 255, 255);
    renderer.drawRect(saveButtonArea);

    renderer.setDrawColor(0, 255, 255, 255);
    renderer.drawRect(loadButtonArea);

    renderer.setDrawColor(r, g, b, a);
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

bool MapEditor::isPointOverAnyPanel(int x, int y) const {
    if (palettePanelExpanded && isPointInRect(x, y, paletteArea)) {
        return true;
    }

    if (layersPanelExpanded && isPointInRect(x, y, layerButtonsArea)) {
        return true;
    }

    if (toolsPanelExpanded && isPointInRect(x, y, toolButtonsArea)) {
        return true;
    }

    if (propertiesPanelExpanded && isPointInRect(x, y, propertiesArea)) {
        return true;
    }

    return false;
}