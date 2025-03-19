#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "tilemap.h"
#include "renderer.h"
#include "imgui/imgui.h"

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
    void renderImGuiInterface();
    void renderToolsPanel();
    void renderLayersPanel();
    void renderTilePalette();
    void renderPropertiesPanel();
    void renderMapBrowser();
    void renderStatusBar();

    void applyTileAtPosition(int gridX, int gridY);
    void eraseTileAtPosition(int gridX, int gridY);
    void openPropertyEditor(int gridX, int gridY);

    std::string currentMapName;
    std::vector<std::string> availableMaps;
    bool showMapBrowser;
    char inputMapNameBuffer[256];
    bool isNamingMap;
    bool isSelectingMap;

    void refreshMapList();
    std::string getMapPath(const std::string& mapName);
    void scanMapDirectory();

    int selectedTileX;
    int selectedTileY;
    bool hasTileSelected;

    bool showDemoWindow;
    bool showAboutWindow;
    bool showMetricsWindow;
    ImVec4 gridColor;
    ImVec4 selectedTileColor;
    ImVec4 cursorColor;
    float gridOpacity;

    bool isMouseButtonDown;
    bool isCtrlPressed;

    struct TileAction {
        int gridX, gridY;
        EditorLayer layer;
        std::string oldTextureID;
        std::string newTextureID;
        bool oldWalkable;
        bool newWalkable;
        std::string oldObjectTexture;
        std::string newObjectTexture;
    };

    std::vector<TileAction> undoStack;
    std::vector<TileAction> redoStack;

    void recordAction(const TileAction& action);
    void undo();
    void redo();
    void selectAll();
};

#endif // MAP_EDITOR_H