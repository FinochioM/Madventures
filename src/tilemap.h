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
    int getGridHeight() const { return gridHeight; }
    int getTileSize() const { return tileSize; }

    void pixelToGrid(int pixelX, int pixelY, int& gridX, int& gridY) const;
    void gridToPixel(int gridX, int gridY, int& pixelX, int& pixelY) const;

    Tile* getTileAt(int gridX, int gridY);

    bool isValidGridPosition(int gridX, int gridY) const;
    bool isWalkable(int gridX, int gridY) const;

    std::vector<std::pair<int, int>> findPath(int startX, int startY, int endX, int endY) const;

private:
    int tileSize;
    int windowWidth;
    int windowHeight;
    int gridWidth;
    int gridHeight;

    std::vector<std::vector<Tile*>> tiles;

    struct PathNode {
        int x, y;
        int gCost; // dist from start.
        int hCost; // dist to end.
        int fCost() const { return gCost + hCost; }
        PathNode* parent;

        PathNode(int x, int y) : x(x), y(y), gCost(0), hCost(0), parent(nullptr) {}
    };

    int calculateHeuristic(int x1, int y1, int x2, int y2) const;
    bool isInList(const std::vector<PathNode*>& list, int x, int y) const;
    PathNode* getNodeFromList(std::vector<PathNode*>& list, int x, int y) const; 
};

#endif // TILEMAP_H