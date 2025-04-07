#include "ui_button.h"

UIButton::UIButton(int x, int y, int width, int height,
                 const std::string& text, const std::string& textureID,
                 UIAnchor anchor)
    : UIElement(x, y, width, height, anchor),
      text(text), textureID(textureID),
      hoverTextureID(""), pressedTextureID(""),
      onClick(nullptr), hovered(false), pressed(false) {
}

UIButton::~UIButton() {
    // Button destructor
}

void UIButton::render(Renderer& renderer) {
    if (!visible) return;

    std::string currentTexture = textureID;

    if (pressed && !pressedTextureID.empty()) {
        currentTexture = pressedTextureID;
    } else if (hovered && !hoverTextureID.empty()) {
        currentTexture = hoverTextureID;
    }

    if (!currentTexture.empty()) {
        renderer.renderTexture(currentTexture, bounds.x, bounds.y, bounds.w, bounds.h);
    } else {
        if (pressed) {
            renderer.setDrawColor(180, 180, 180, 255);
        } else if (hovered) {
            renderer.setDrawColor(220, 220, 220, 255);
        } else {
            renderer.setDrawColor(200, 200, 200, 255);
        }
        renderer.fillRect(bounds);

        renderer.setDrawColor(100, 100, 100, 255);
        renderer.drawRect(bounds);
    }

    if (!text.empty()) {
        int textX = bounds.x + (bounds.w / 2) - (text.length() * 8 / 2);
        int textY = bounds.y + (bounds.h / 2) - 8;
        renderer.drawText(text, textX, textY);
    }
}

bool UIButton::handleEvent(SDL_Event& e) {
    if (!active) return false;

    if (e.type == SDL_MOUSEMOTION) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        bool wasHovered = hovered;
        hovered = containsPoint(mouseX, mouseY);

        return wasHovered != hovered;
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (containsPoint(mouseX, mouseY)) {
                pressed = true;
                return true;
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT && pressed) {
            pressed = false;

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (containsPoint(mouseX, mouseY) && onClick) {
                onClick();
                return true;
            }

            return true;
        }
    }

    return false;
}

void UIButton::setOnClick(std::function<void()> callback) {
    onClick = callback;
}

void UIButton::setText(const std::string& text) {
    this->text = text;
}

void UIButton::setTextureID(const std::string& textureID) {
    this->textureID = textureID;
}

void UIButton::setHoverTextureID(const std::string& textureID) {
    this->hoverTextureID = textureID;
}

void UIButton::setPressedTextureID(const std::string& textureID) {
    this->pressedTextureID = textureID;
}