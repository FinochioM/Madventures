#ifndef TILEMAP_H
#define TILEMAP_H

#include <vector>
#include "tile.h"
#include "renderer.h"

class TileMap {
public:
    TileMap(int tileSize, int windowWidth, int windowHeight);
    ~TileMap();

    void initialize();

    void render(Renderer& renderer);

    int getGridWidth() const { return gridWidth; }
    int getGridHeihgt() const { return gridHeight; }
    int getTileSize() const { return tileSize; }

    void pixelToGrid(int pixelX, int pixelY, int& gridX, int& gridY) const;
    void gridToPixel(int gridX, int gridY, int& pixelX, int& pixelY) const;

    Tile* getTileAt(int gridX, int gridY);

    bool isValidGridPosition(int gridX, int gridY) const;

private:
    int tileSize;
    int windowWidth;
    int windowHeight;
    int gridWidth;
    int gridHeight;

    std::vector<std::vector<Tile*>> tiles;
};

#endif // TILEMAP_H