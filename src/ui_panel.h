#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "ui_element.h"
#include <vector>

class UIPanel : public UIElement {
public:
    UIPanel(int x, int y, int width, int height,
           const std::string& textureID = "",
           UIAnchor anchor = UIAnchor::TOP_LEFT);
    ~UIPanel();

    void render(Renderer& renderer) override;
    bool handleEvent(SDL_Event& e) override;

    void setTextureID(const std::string& textureID);
    void addElement(UIElement* element);

private:
    std::string textureID;
    std::vector<UIElement*> elements;
};

#endif //UI_PANEL_H