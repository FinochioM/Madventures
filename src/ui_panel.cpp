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
        int childX = bounds.x + element->getX();
        int childY = bounds.y + element->getY();

        int originalX = element->getX();
        int originalY = element->getY();

        element->setPosition(childX, childY);
        element->render(renderer);

        element->setPosition(originalX, originalY);
    }
}


bool UIPanel::handleEvent(SDL_Event& e) {
    if (!active) return false;

    int mouseX = 0, mouseY = 0;
    if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        SDL_GetMouseState(&mouseX, &mouseY);

        if (!containsPoint(mouseX, mouseY)) {
            return false;
        }
    }

    SDL_Event relativeEvent = e;

    if (e.type == SDL_MOUSEMOTION) {
        relativeEvent.motion.x = mouseX - bounds.x;
        relativeEvent.motion.y = mouseY - bounds.y;
    } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        relativeEvent.button.x = mouseX - bounds.x;
        relativeEvent.button.y = mouseY - bounds.y;
    }

    for (auto element : elements) {
        int originalX = element->getX();
        int originalY = element->getY();
        SDL_Rect originalBounds = element->getBounds();

        element->setPosition(originalX, originalY);

        bool handled = element->handleEvent(relativeEvent);

        element->setPosition(originalX, originalY);

        if (handled) {
            return true;
        }
    }

    return false;
}

void UIPanel::setTextureID(const std::string& textureID) {
    this->textureID = textureID;
}

void UIPanel::addElement(UIElement* element) {
    elements.push_back(element);
}
