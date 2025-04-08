#ifndef UI_EDITOR_H
#define UI_EDITOR_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <map>
#include "imgui/imgui.h"
#include "renderer.h"
#include "ui_manager.h"
#include "ui_element.h"
#include "ui_button.h"
#include "ui_label.h"
#include "ui_panel.h"

class UIEditor {
public:
    UIEditor(UIManager* cityUIManager, UIManager* arenaUIManager);
    ~UIEditor();

    void handleEvent(SDL_Event& e);
    void update();
    void render(Renderer& renderer);

    bool isActive() const { return active; }
    void setActive(bool active) { this->active = active; }
    void toggleActive() { active = !active; }

    bool saveLayout(const std::string& filename);
    bool loadLayout(const std::string& filename);

private:
    UIManager* cityUIManager;
    UIManager* arenaUIManager;
    UIManager* currentUIManager;

    bool active;
    bool draggingElement;
    bool resizingElement;
    UIElement* selectedElement;
    int dragOffsetX, dragOffsetY;
    int resizeStartWidth, resizeStartHeight;
    int resizeStartX, resizeStartY;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;

    void renderImGuiInterface();
    void renderElementsPanel();
    void renderPropertiesPanel();
    void renderToolsPanel();
    void renderStatusBar();

    UIElement* findElementAt(int x, int y);
    void highlightElement(UIElement* element, Renderer& renderer);
    void startDragging(UIElement* element, int mouseX, int mouseY);
    void startResizing(UIElement* element, int mouseX, int mouseY);
    void updateDragging(int mouseX, int mouseY);
    void updateResizing(int mouseX, int mouseY);

    enum class ResizeHandle {
        NONE,
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };

    ResizeHandle getResizeHandleAt(UIElement* element, int x, int y);
    ResizeHandle currentResizeHandle;

    std::string currentLayout;
    std::vector<std::string> availableLayouts;
    char inputLayoutNameBuffer[256];
    bool isNamingLayout;
    bool showLayoutBrowser;

    void refreshLayoutList();
    std::string getLayoutPath(const std::string& layoutName);

    struct UIElementInfo {
        std::string id;
        std::string type;
        UIElement* element;

        UIElementInfo(const std::string& id, const std::string& type, UIElement* element)
            : id(id), type(type), element(element) {}
    };

    std::vector<UIElementInfo> elementInfos;
    void updateElementInfos();

    enum class UIManagerType {
        CITY,
        ARENA
    };

    UIManagerType currentManager;
    void switchUIManager(UIManagerType type);
};

#endif // UI_EDITOR_H