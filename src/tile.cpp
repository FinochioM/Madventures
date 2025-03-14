#include "tile.h"
#include "tilemap.h"

// global for now
extern TileMap* g_TileMap;

Tile::Tile(int gridX, int gridY) : gridX(gridX), gridY(gridY) {
    // initialize defaults ?
}

Tile::~Tile() {
    // destructor
}

int Tile::getPixelX() const {
    return gridX * g_TileMap->getTileSize();
}

int Tile::getPixelY() const {
    return gridY * g_TileMap->getTileSize();
}