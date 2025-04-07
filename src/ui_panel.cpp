#include "ui_panel.h"

UIPanel::UIPanel(int x, int y, int width, int height,
                const std::string& textureID,
                UIAnchor anchor)
    : UIElement(x, y, width, height, anchor),
      textureID(textureID) {
}

UIPanel::~UIPanel() {
    for (auto element : elements) {
        delete element;
    }
    elements.clear();
}

void UIPanel::render(Renderer& renderer) {
    if (!visible) return;

    if (!textureID.empty()) {
        renderer.renderTexture(textureID, bounds.x, bounds.y, bounds.w, bounds.h);
    }

    for (auto element : elements) {
        element->render(renderer);
    }
}

bool UIPanel::handleEvent(SDL_Event& e) {
    if (!active) return false;

    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true;
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        return containsPoint(mouseX, mouseY);
    }

    return false;
}

void UIPanel::setTextureID(const std::string& textureID) {
    this->textureID = textureID;
}

void UIPanel::addElement(UIElement* element) {
    elements.push_back(element);
}
