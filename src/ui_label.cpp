#include "ui_label.h"

UILabel::UILabel(int x, int y, const std::string& text, UIAnchor anchor)
    : UIElement(x, y, text.length() * 8, 16, anchor),
      text(text) {
    textColor = {255, 255, 255, 255};
}

UILabel::~UILabel() {
    // Label destructor
}

void UILabel::render(Renderer& renderer) {
    if (!visible) return;

    renderer.drawText(text, bounds.x, bounds.y);
}

bool UILabel::handleEvent(SDL_Event& e) {
    return false;
}

void UILabel::setText(const std::string& text) {
    this->text = text;
    setSize(text.length() * 8, 16);
}

void UILabel::setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    textColor.r = r;
    textColor.g = g;
    textColor.b = b;
    textColor.a = a;
}