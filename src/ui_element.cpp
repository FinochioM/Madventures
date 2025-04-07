#include "ui_element.h"

UIElement::UIElement(int x, int y, int width, int height, UIAnchor anchor)
    : x(x), y(y), width(width), height(height), anchor(anchor),
      visible(true), active(true) {
    updateBounds();
}

UIElement::~UIElement() {
    // Base destructor
}

void UIElement::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
    updateBounds();
}

void UIElement::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    updateBounds();
}

void UIElement::setAnchor(UIAnchor anchor) {
    this->anchor = anchor;
    updateBounds();
}

void UIElement::setVisible(bool visible) {
    this->visible = visible;
}

void UIElement::setActive(bool active) {
    this->active = active;
}

SDL_Rect UIElement::getBounds() const {
    return bounds;
}

bool UIElement::containsPoint(int pointX, int pointY) const {
    return (pointX >= bounds.x && pointX <= bounds.x + bounds.w &&
            pointY >= bounds.y && pointY <= bounds.y + bounds.h);
}

void UIElement::calculateActualPosition(int screenWidth, int screenHeight) {
    int actualX = x;
    int actualY = y;

    switch (anchor) {
        case UIAnchor::TOP_CENTER:
        case UIAnchor::MIDDLE_CENTER:
        case UIAnchor::BOTTOM_CENTER:
            actualX = (screenWidth - width) / 2 + x;
            break;

        case UIAnchor::TOP_RIGHT:
        case UIAnchor::MIDDLE_RIGHT:
        case UIAnchor::BOTTOM_RIGHT:
            actualX = screenWidth - width - x;
            break;

        default:
            break;
    }

    switch (anchor) {
        case UIAnchor::MIDDLE_LEFT:
        case UIAnchor::MIDDLE_CENTER:
        case UIAnchor::MIDDLE_RIGHT:
            actualY = (screenHeight - height) / 2 + y;
            break;

        case UIAnchor::BOTTOM_LEFT:
        case UIAnchor::BOTTOM_CENTER:
        case UIAnchor::BOTTOM_RIGHT:
            actualY = screenHeight - height - y;
            break;

        default:
            break;
    }

    bounds.x = actualX;
    bounds.y = actualY;
}

void UIElement::updateBounds() {
    bounds.x = x;
    bounds.y = y;
    bounds.w = width;
    bounds.h = height;
}