#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <SDL2/SDL.h>
#include <string>
#include "renderer.h"

enum class UIAnchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

class UIElement {
public:
    UIElement(int x, int y, int width, int height, UIAnchor anchor = UIAnchor::TOP_LEFT);
    virtual ~UIElement();

    virtual void render(Renderer& renderer) = 0;
    virtual bool handleEvent(SDL_Event& e) = 0;

    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setAnchor(UIAnchor anchor);
    void setVisible(bool visible);
    void setActive(bool active);

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isVisible() const { return visible; }
    bool isActive() const { return active; }

    SDL_Rect getBounds() const;
    bool containsPoint(int pointX, int pointY) const;

    void calculateActualPosition(int screenWidth, int screenHeight);

protected:
    int x, y;                   // Position relative to anchor
    int width, height;          // Element dimensions
    UIAnchor anchor;            // Positioning anchor
    bool visible;               // Is element visible
    bool active;                // Is element interactive
    SDL_Rect bounds;            // Actual screen position and size

    void updateBounds();
};

#endif // UI_ELEMENT_H