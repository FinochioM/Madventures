#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "ui_element.h"
#include <functional>

class UIButton : public UIElement {
public:
    UIButton(int x, int y, int width, int height,
             const std::string& text, const std::string& textureID = "",
             UIAnchor anchor = UIAnchor::TOP_LEFT);
    ~UIButton();

    void render(Renderer& renderer) override;
    bool handleEvent(SDL_Event& e) override;

    void setOnClick(std::function<void()> callback);
    void setText(const std::string& text);
    void setTextureID(const std::string& textureID);
    void setHoverTextureID(const std::string& textureID);
    void setPressedTextureID(const std::string& textureID);

private:
    std::string text;
    std::string textureID;
    std::string hoverTextureID;
    std::string pressedTextureID;
    std::function<void()> onClick;
    bool hovered;
    bool pressed;
};

#endif // UI_BUTTON_H