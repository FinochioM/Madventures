#include "ui_manager.h"

UIManager::UIManager(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight) {
}

UIManager::~UIManager() {
    clear();
}

void UIManager::render(Renderer& renderer) {
    for (auto element : elements) {
        element->calculateActualPosition(screenWidth, screenHeight);
        element->render(renderer);
    }
}

bool UIManager::handleEvent(SDL_Event& e) {
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true;
        }
    }

    return false;
}

void UIManager::addElement(UIElement* element) {
    elements.push_back(element);
    element->calculateActualPosition(screenWidth, screenHeight);
}

void UIManager::removeElement(UIElement* element) {
    auto it = std::find(elements.begin(), elements.end(), element);
    if (it != elements.end()) {
        elements.erase(it);
    }
}

void UIManager::clear() {
    for (auto element : elements) {
        delete element;
    }
    elements.clear();
}

void UIManager::setScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    for (auto element : elements) {
        element->calculateActualPosition(screenWidth, screenHeight);
    }
}

UIElement* UIManager::findElementById(const std::string& id) {
    // This is a simple implementation - in a real scenario we'd need
    // to store element IDs somewhere or make UIElement have an ID property

    // For now, just return nullptr as a placeholder
    return nullptr;
}