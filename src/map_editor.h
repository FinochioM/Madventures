#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "tilemap.h"
#include "renderer.h"

enum class EditorTool {
    PENCIL,
    ERASER,
    PROPERTY_EDITOR
};

enum class EditorLayer {
    GROUND,
    OBJECTS,
    COLLISION
};

struct TileTexture {
    std::string id;
    std::string name;
    bool walkable;
};

class MapEditor {
public:
    MapEditor(TileMap* tileMap);
    ~MapEditor();

    void handleEvent(SDL_Event& e);
    void update();
    void render(Renderer& renderer);

    bool isActive() const { return active; }
    void setActive(bool active) { this->active = active; }
    void toggleActive() { active = !active; }

    bool loadMap(const std::string& filename);
    bool saveMap(const std::string& filename);

private:
    TileMap* tileMap;
    bool active;
    int mouseX, mouseY;
    int gridX, gridY;
    EditorTool currentTool;
    EditorLayer currentLayer;
    int currentTileIndex;

    std::vector<TileTexture> availableTiles;

    SDL_Rect paletteArea;
    SDL_Rect layerButtonsArea;
    SDL_Rect toolButtonsArea;
    SDL_Rect propertiesArea;

    bool showPropertyPanel;
    std::string editingProperty;
    bool editingPropertyValue;

    void initializeAvailableTiles();
    void renderPalette(Renderer& renderer);
    void renderLayerButtons(Renderer& renderer);
    void renderToolButtons(Renderer& renderer);
    void renderPropertyPanel(Renderer& renderer);

    void handleLeftClick(int mouseX, int mouseY);
    void handleRightClick(int mouseX, int mouseY);

    void applyTileAtPosition(int gridX, int gridY);
    void eraseTileAtPosition(int gridX, int gridY);
    void openPropertyEditor(int gridX, int gridY);

    bool isPointInRect(int x, int y, const SDL_Rect& rect) const;

    std::string currentMapName;
    std::vector<std::string> availableMaps;
    bool showMapBrowser;
    std::string inputMapName;
    bool isNamingMap;
    bool isSelectingMap;

    SDL_Rect saveButtonArea;
    SDL_Rect loadButtonArea;
    SDL_Rect inputArea;
    SDL_Rect mapListArea;

    void renderMapBrowser(Renderer& renderer);
    void refreshMapList();
    std::string getMapPath(const std::string& mapName);

    void drawUIBounds(Renderer& renderer);

    bool palettePanelExpanded;
    bool layersPanelExpanded;
    bool toolsPanelExpanded;
    bool propertiesPanelExpanded;

    SDL_Rect palettePanelHeaderArea;
    SDL_Rect layersPanelHeaderArea;
    SDL_Rect toolsPanelHeaderArea;
    SDL_Rect propertiesPanelHeaderArea;

    void scanMapDirectory();

    int selectedTileX;
    int selectedTileY;
    bool hasTileSelected;

    bool isPointOverAnyPanel(int x, int y) const;
};

#endif // MAP_EDITOR_H