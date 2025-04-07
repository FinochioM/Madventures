#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "ui_element.h"
#include <vector>
#include <algorithm>

class UIManager {
public:
    UIManager(int screenWidth, int screenHeight);
    ~UIManager();

    void render(Renderer& renderer);
    bool handleEvent(SDL_Event& e);

    void addElement(UIElement* element);
    void removeElement(UIElement* element);
    void clear();

    void setScreenSize(int width, int height);

private:
    int screenWidth, screenHeight;
    std::vector<UIElement*> elements;
};

#endif //UI_MANAGER_H