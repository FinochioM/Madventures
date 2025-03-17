#include "tilemap.h"
#include <iostream>
#include <algorithm>
#include <cmath>

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

            tiles[y][x]->setProperty("walkable", true);

            tiles[y][x]->setProperty("textureID", "tile_grass");
        }
    }

    std::cout << "Empty TileMap initialized with " << gridWidth << "x" << gridHeight << " tiles (" << tileSize << "px each)" << std::endl;
}

void TileMap::render(Renderer& renderer) {
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            Tile* tile = tiles[y][x];
            if (tile) {
                int pixelX = x * tileSize;
                int pixelY = y * tileSize;

                std::string textureID = tile->getProperty<std::string>("textureID", "");
                if (!textureID.empty()) {
                    renderer.renderTexture(textureID, pixelX, pixelY, tileSize, tileSize);
                } else {
                    renderer.setDrawColor(40, 40, 40, 255);
                    renderer.fillRect(pixelX, pixelY, tileSize, tileSize);
                }

                std::string objectTextureID = tile->getProperty<std::string>("objectTexture", "");
                if (!objectTextureID.empty()) {
                    renderer.renderTexture(objectTextureID, pixelX, pixelY, tileSize, tileSize);
                }

                if (!tile->getProperty("walkable", true)) {
                    renderer.setDrawColor(255, 0, 0, 100);
                    renderer.drawRect(pixelX, pixelY, tileSize, tileSize);
                }
            }
        }
    }
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
    return gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight;
}

bool TileMap::isWalkable(int gridX, int gridY) const {
    if (!isValidGridPosition(gridX, gridY)) {
        return false;
    }

    return tiles[gridY][gridX]->getProperty("walkable", false);
}

std::vector<std::pair<int, int>> TileMap::findPath(int startX, int startY, int endX, int endY) const {
    std::vector<std::pair<int, int>> path;

    if (!isValidGridPosition(startX, startY) || !isValidGridPosition(endX, endY) ||
        !tiles[startY][startX]->getProperty("walkable", false) ||
        !tiles[endY][endX]->getProperty("walkable", false)) {
                return path;
        }

    std::vector<PathNode*> openList;
    std::vector<PathNode*> closedList;

    PathNode* startNode = new PathNode(startX, startY);
    PathNode* endNode = new PathNode(endX, endY);

    startNode->hCost = calculateHeuristic(startX, startY, endX, endY);
    openList.push_back(startNode);

    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};

    while (!openList.empty()) {
        auto currentIt = std::min_element(openList.begin(), openList.end(),
            [](const PathNode* a, const PathNode* b) {
                return a->fCost() < b->fCost() ||
                    (a->fCost() == b->fCost() && a->hCost < b->hCost);
            });

        PathNode* current = *currentIt;

        if (current->x == endX && current->y == endY) {
            PathNode* temp = current;

            while (temp != nullptr) {
                path.push_back(std::make_pair(temp->x, temp->y));
                temp = temp->parent;
            }

            std::reverse(path.begin(), path.end());

            // cleanup
            for (auto node : openList) delete node;
            for (auto node : closedList) delete node;

            return path;
        }

        openList.erase(currentIt);
        closedList.push_back(current);

        for (int i = 0; i < 4; i++) {
            int neighborX = current->x + dx[i];
            int neighborY = current->y + dy[i];

            if (!isValidGridPosition(neighborX, neighborY) ||
                !tiles[neighborY][neighborX]->getProperty("walkable", false) ||
                isInList(closedList, neighborX, neighborY)) {
                    continue;
            }

            int newGCost = current->gCost + 10; // test later

            PathNode* neighborNode = getNodeFromList(openList, neighborX, neighborY);

            if (neighborNode == nullptr) {
                // new node
                neighborNode = new PathNode(neighborX, neighborY);
                neighborNode->gCost = newGCost;
                neighborNode->hCost = calculateHeuristic(neighborX, neighborY, endX, endY);
                neighborNode->parent = current;
                openList.push_back(neighborNode);
            } else if (newGCost < neighborNode->gCost) {
                neighborNode->gCost = newGCost;
                neighborNode->parent = current;
            }
        }
    }

    //cleanup all nodes
    for (auto node : openList) delete node;
    for (auto node : closedList) delete node;

    // no path found.
    return path;
}

int TileMap::calculateHeuristic(int x1, int y1, int x2, int y2) const {
    return std::abs(x2 - x1) + std::abs(y2 - y1);
}

bool TileMap::isInList(const std::vector<PathNode*>& list, int x, int y) const {
    return std::find_if(list.begin(), list.end(), [x, y](const PathNode* node){
        return node->x == x && node->y == y;
    }) != list.end();
}

TileMap::PathNode* TileMap::getNodeFromList(std::vector<PathNode*>& list, int x, int y) const {
    auto it = std::find_if(list.begin(), list.end(), [x, y](const PathNode* node) {
        return node->x == x && node->y == y;
    });

    return (it != list.end()) ? *it : nullptr;
}

void TileMap::setTileTexture(int gridX, int gridY, const std::string& textureID) {
    if (isValidGridPosition(gridX, gridY)) {
        tiles[gridY][gridX]->setProperty("textureID", textureID);
    }
}