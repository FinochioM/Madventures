#include "tilemap.h"
#include <iostream>

// global for now

TileMap* g_TileMap = nullptr;

TileMap::TileMap(int tileSize, int windowWidth, int windowHeight)
    : tileSize(tileSize), windowWidth(windowWidth), windowHeight(windowHeight) {

    gridWidth = windowWidth / tileSize;
    gridHeight = windowHeight / tileSize;

    g_TileMap = this;
}

TileMap::~TileMap() {
    for (auto& row : tiles) {
        for (auto& tile : row) {
            delete tile;
        }
    }

    if (g_TileMap == this) {
        g_TileMap = nullptr;
    }
}

void TileMap::initialize() {
    tiles.resize(gridHeight);

    for (int y = 0; y < gridHeight; y++) {
        tiles[y].resize(gridWidth);

        for (int x = 0; x < gridWidth; x++) {
            tiles[y][x] = new Tile(x, y);

            // default properties should go here ...
            // maybe like tiles[y][x]->setProperty("name", value);
        }
    }

    std::cout << "TileMap initialized with " << gridWidth << "x" << gridHeight << " tiles (" << tileSize << "px each)" << std::endl;
}

void TileMap::render(Renderer& renderer) {
    // should load textures here too.
    // lines for now

    renderer.setDrawColor(50, 50, 50, 255);

    for (int y = 0; y <= gridHeight; y++) {
        int pixelY = y * tileSize;
        renderer.drawRect(0, pixelY, windowWidth, 1);
    }

    for (int x = 0; x <= gridWidth; x++) {
        int pixelX = x * tileSize;
        renderer.drawRect(pixelX, 0, 1, windowHeight);
    }

    // i could render different tiles here depending on their properties, dont know how tho.
}

void TileMap::pixelToGrid(int pixelX, int pixelY, int& gridX, int& gridY) const {
    gridX = pixelX / tileSize;
    gridY = pixelY / tileSize;
}

void TileMap::gridToPixel(int gridX, int gridY, int& pixelX, int& pixelY) const {
    pixelX = gridX * tileSize;
    pixelY = gridY * tileSize;
}

Tile* TileMap::getTileAt(int gridX, int gridY) {
    if (isValidGridPosition(gridX, gridY)) {
        return tiles[gridY][gridX];
    }

    return nullptr;
}

bool TileMap::isValidGridPosition(int gridX, int gridY) const {
    return gridX >= 0 && gridY < gridWidth && gridY >= 0 && gridY < gridHeight;
}