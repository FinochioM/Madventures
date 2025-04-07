#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "ui_element.h"
#include <string>

class UILabel : public UIElement {
public:
    UILabel(int x, int y, const std::string& text, UIAnchor anchor = UIAnchor::TOP_LEFT);
    ~UILabel();

    void render(Renderer& renderer) override;
    bool handleEvent(SDL_Event& e) override;

    void setText(const std::string& text);
    void setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

private:
    std::string text;
    SDL_Color textColor;
};

#endif //UI_LABEL_H