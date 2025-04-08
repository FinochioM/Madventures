#include "ui_editor.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <json.hpp>

UIEditor::UIEditor(UIManager* cityUIManager, UIManager* arenaUIManager)
    : cityUIManager(cityUIManager),
      arenaUIManager(arenaUIManager),
      currentUIManager(cityUIManager),
      active(false),
      draggingElement(false),
      resizingElement(false),
      selectedElement(nullptr),
      dragOffsetX(0), dragOffsetY(0),
      resizeStartWidth(0), resizeStartHeight(0),
      resizeStartX(0), resizeStartY(0),
      mouseX(0), mouseY(0),
      lastMouseX(0), lastMouseY(0),
      currentResizeHandle(ResizeHandle::NONE),
      currentLayout("default"),
      isNamingLayout(false),
      showLayoutBrowser(false),
      currentManager(UIManagerType::CITY) {

    strcpy(inputLayoutNameBuffer, currentLayout.c_str());
    refreshLayoutList();
    updateElementInfos();
}

UIEditor::~UIEditor() {
    // Nothing to clean up
}

void UIEditor::handleEvent(SDL_Event& e) {
    if (!active) return;

    if (e.type == SDL_MOUSEMOTION) {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        mouseX = e.motion.x;
        mouseY = e.motion.y;

        if (draggingElement && selectedElement) {
            updateDragging(mouseX, mouseY);
        }

        if (resizingElement && selectedElement) {
            updateResizing(mouseX, mouseY);
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                UIElement* element = findElementAt(mouseX, mouseY);

                if (element) {
                    selectedElement = element;

                    ResizeHandle handle = getResizeHandleAt(selectedElement, mouseX, mouseY);
                    if (handle != ResizeHandle::NONE) {
                        startResizing(selectedElement, mouseX, mouseY);
                        currentResizeHandle = handle;
                    } else {
                        startDragging(selectedElement, mouseX, mouseY);
                    }
                } else {
                    selectedElement = nullptr;
                }
            }
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            draggingElement = false;
            resizingElement = false;
            currentResizeHandle = ResizeHandle::NONE;
        }
    }

    if (e.type == SDL_KEYDOWN) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            if (e.key.keysym.sym == SDLK_s && (SDL_GetModState() & KMOD_CTRL)) {
                isNamingLayout = true;
                showLayoutBrowser = true;
                strcpy(inputLayoutNameBuffer, currentLayout.c_str());
            }

            if (e.key.keysym.sym == SDLK_o && (SDL_GetModState() & KMOD_CTRL)) {
                showLayoutBrowser = true;
                isNamingLayout = false;
                refreshLayoutList();
            }

            if (e.key.keysym.sym == SDLK_1) {
                switchUIManager(UIManagerType::CITY);
            }
            else if (e.key.keysym.sym == SDLK_2) {
                switchUIManager(UIManagerType::ARENA);
            }

            // Delete selected element (be careful - this would need proper element management!)
            /*
            if (e.key.keysym.sym == SDLK_DELETE && selectedElement) {
                // Remove from manager - this would need more structure to work safely
                selectedElement = nullptr;
            }
            */
        }
    }
}

void UIEditor::update() {
    if (!active) return;

    // Any ongoing updates here...
}

void UIEditor::render(Renderer& renderer) {
    if (!active) return;

    if (selectedElement) {
        highlightElement(selectedElement, renderer);
    }

    renderImGuiInterface();
}

void UIEditor::renderImGuiInterface() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Layout")) {
                // Clear current layout
            }

            if (ImGui::MenuItem("Open Layout", "Ctrl+O")) {
                showLayoutBrowser = true;
                isNamingLayout = false;
                refreshLayoutList();
            }

            if (ImGui::MenuItem("Save Layout", "Ctrl+S")) {
                saveLayout(getLayoutPath(currentLayout));
            }

            if (ImGui::MenuItem("Save Layout As...")) {
                isNamingLayout = true;
                showLayoutBrowser = true;
                strcpy(inputLayoutNameBuffer, currentLayout.c_str());
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit Editor")) {
                setActive(false);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("City UI", "1", currentManager == UIManagerType::CITY)) {
                switchUIManager(UIManagerType::CITY);
            }

            if (ImGui::MenuItem("Arena UI", "2", currentManager == UIManagerType::ARENA)) {
                switchUIManager(UIManagerType::ARENA);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    renderElementsPanel();

    renderPropertiesPanel();

    renderToolsPanel();

    renderStatusBar();

    if (showLayoutBrowser) {
        ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);

        const char* title = isNamingLayout ? "Save Layout As" : "Open Layout";

        ImGui::Begin(title, &showLayoutBrowser, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (isNamingLayout) {
            ImGui::Text("Enter Layout Name:");

            if (ImGui::InputText("##LayoutName", inputLayoutNameBuffer, sizeof(inputLayoutNameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (strlen(inputLayoutNameBuffer) > 0) {
                    currentLayout = inputLayoutNameBuffer;
                    saveLayout(getLayoutPath(currentLayout));
                    showLayoutBrowser = false;
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Save", ImVec2(120, 0))) {
                if (strlen(inputLayoutNameBuffer) > 0) {
                    currentLayout = inputLayoutNameBuffer;
                    saveLayout(getLayoutPath(currentLayout));
                    showLayoutBrowser = false;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                showLayoutBrowser = false;
            }
        } else {
            ImGui::Text("Select a Layout:");
            ImGui::Separator();

            for (const auto& layoutName : availableLayouts) {
                if (ImGui::Selectable(layoutName.c_str(), currentLayout == layoutName)) {
                    currentLayout = layoutName;
                    loadLayout(getLayoutPath(currentLayout));
                    showLayoutBrowser = false;
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                showLayoutBrowser = false;
            }
        }

        ImGui::End();
    }
}

void UIEditor::renderElementsPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("UI Elements", nullptr)) {
        if (ImGui::TreeNode("Elements")) {
            for (const auto& info : elementInfos) {
                bool isSelected = (selectedElement == info.element);
                if (ImGui::Selectable(info.id.c_str(), isSelected)) {
                    selectedElement = info.element;
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    // Center view on this element
                }
            }
            ImGui::TreePop();
        }

        ImGui::Separator();

        // This would require a way to create and add new elements
        /*
        if (ImGui::Button("Add Button", ImVec2(180, 0))) {
            // Add button logic
        }

        if (ImGui::Button("Add Label", ImVec2(180, 0))) {
            // Add label logic
        }

        if (ImGui::Button("Add Panel", ImVec2(180, 0))) {
            // Add panel logic
        }
        */
    }
    ImGui::End();
}

void UIEditor::renderPropertiesPanel() {
    ImGui::SetNextWindowPos(ImVec2(600, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Properties", nullptr)) {
        if (selectedElement) {
            std::string elementType = "Unknown";
            for (const auto& info : elementInfos) {
                if (info.element == selectedElement) {
                    elementType = info.type;
                    ImGui::Text("ID: %s", info.id.c_str());
                    break;
                }
            }

            ImGui::Text("Type: %s", elementType.c_str());
            ImGui::Separator();

            int x = selectedElement->getX();
            int y = selectedElement->getY();
            int width = selectedElement->getWidth();
            int height = selectedElement->getHeight();

            if (ImGui::DragInt("X", &x, 1.0f)) {
                selectedElement->setPosition(x, y);
            }

            if (ImGui::DragInt("Y", &y, 1.0f)) {
                selectedElement->setPosition(x, y);
            }

            if (ImGui::DragInt("Width", &width, 1.0f)) {
                selectedElement->setSize(width, height);
            }

            if (ImGui::DragInt("Height", &height, 1.0f)) {
                selectedElement->setSize(width, height);
            }

            ImGui::Separator();

            const char* anchorTypes[] = {
                "TOP_LEFT", "TOP_CENTER", "TOP_RIGHT",
                "MIDDLE_LEFT", "MIDDLE_CENTER", "MIDDLE_RIGHT",
                "BOTTOM_LEFT", "BOTTOM_CENTER", "BOTTOM_RIGHT"
            };

            // We'd need to add a method to get current anchor
            // int currentAnchor = static_cast<int>(selectedElement->getAnchor());
            int currentAnchor = 0;

            if (ImGui::Combo("Anchor", &currentAnchor, anchorTypes, IM_ARRAYSIZE(anchorTypes))) {
                selectedElement->setAnchor(static_cast<UIAnchor>(currentAnchor));
            }

            ImGui::Separator();

            bool visible = selectedElement->isVisible();
            if (ImGui::Checkbox("Visible", &visible)) {
                selectedElement->setVisible(visible);
            }

            bool active = selectedElement->isActive();
            if (ImGui::Checkbox("Active", &active)) {
                selectedElement->setActive(active);
            }

            ImGui::Separator();

            if (elementType == "Button") {
                UIButton* button = dynamic_cast<UIButton*>(selectedElement);
                if (button) {
                    // Would need to add accessors to UIButton to make this work
                    /*
                    char textBuffer[256];
                    strcpy(textBuffer, button->getText().c_str());
                    if (ImGui::InputText("Text", textBuffer, sizeof(textBuffer))) {
                        button->setText(textBuffer);
                    }

                    char textureBuffer[256];
                    strcpy(textureBuffer, button->getTextureID().c_str());
                    if (ImGui::InputText("Texture ID", textureBuffer, sizeof(textureBuffer))) {
                        button->setTextureID(textureBuffer);
                    }

                    char hoverTextureBuffer[256];
                    strcpy(hoverTextureBuffer, button->getHoverTextureID().c_str());
                    if (ImGui::InputText("Hover Texture", hoverTextureBuffer, sizeof(hoverTextureBuffer))) {
                        button->setHoverTextureID(hoverTextureBuffer);
                    }
                    */
                }
            }
            else if (elementType == "Label") {
                UILabel* label = dynamic_cast<UILabel*>(selectedElement);
                if (label) {
                    // Would need to add accessors to UILabel
                    /*
                    char textBuffer[256];
                    strcpy(textBuffer, label->getText().c_str());
                    if (ImGui::InputText("Text", textBuffer, sizeof(textBuffer))) {
                        label->setText(textBuffer);
                    }
                    */
                }
            }
            else if (elementType == "Panel") {
                UIPanel* panel = dynamic_cast<UIPanel*>(selectedElement);
                if (panel) {
                    // Panel-specific properties
                    /*
                    char textureBuffer[256];
                    strcpy(textureBuffer, panel->getTextureID().c_str());
                    if (ImGui::InputText("Texture ID", textureBuffer, sizeof(textureBuffer))) {
                        panel->setTextureID(textureBuffer);
                    }
                    */
                }
            }
        }
        else {
            ImGui::Text("No element selected");
        }
    }
    ImGui::End();
}

void UIEditor::renderToolsPanel() {
    ImGui::SetNextWindowPos(ImVec2(220, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(180, 200), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Tools", nullptr)) {
        ImGui::Text("UI Editor Tools");
        ImGui::Separator();

        if (ImGui::Button("Select Mode", ImVec2(160, 0))) {
            // Set selection mode
        }

        if (ImGui::Button("Move Mode", ImVec2(160, 0))) {
            // Set move mode
        }

        if (ImGui::Button("Resize Mode", ImVec2(160, 0))) {
            // Set resize mode
        }

        ImGui::Separator();

        if (ImGui::Button("Align Left", ImVec2(75, 0))) {
            // Align selected to left
        }

        ImGui::SameLine();

        if (ImGui::Button("Align Right", ImVec2(75, 0))) {
            // Align selected to right
        }

        if (ImGui::Button("Align Top", ImVec2(75, 0))) {
            // Align selected to top
        }

        ImGui::SameLine();

        if (ImGui::Button("Align Bottom", ImVec2(75, 0))) {
            // Align selected to bottom
        }

        if (ImGui::Button("Center Horizontally", ImVec2(160, 0))) {
            // Center horizontally
        }

        if (ImGui::Button("Center Vertically", ImVec2(160, 0))) {
            // Center vertically
        }
    }
    ImGui::End();
}

void UIEditor::renderStatusBar() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20));
    ImGui::Begin("##StatusBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::Text("Mouse: %d, %d", mouseX, mouseY);

    ImGui::SameLine(150);

    ImGui::Text("Layout: %s", currentLayout.c_str());

    ImGui::SameLine(300);

    const char* managerType = (currentManager == UIManagerType::CITY) ? "City UI" : "Arena UI";
    ImGui::Text("Current UI: %s", managerType);

    ImGui::SameLine(ImGui::GetWindowWidth() - 100);

    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("UI Editor Controls:");
        ImGui::Text("1-2: Switch UI managers");
        ImGui::Text("Ctrl+S: Save layout");
        ImGui::Text("Ctrl+O: Open layout");
        ImGui::Text("Click and drag: Move elements");
        ImGui::Text("Edges: Resize elements");
        ImGui::EndTooltip();
    }

    ImGui::End();
}

UIElement* UIEditor::findElementAt(int x, int y) {
    for (auto it = elementInfos.rbegin(); it != elementInfos.rend(); ++it) {
        if (it->element->containsPoint(x, y) && it->element->isVisible()) {
            return it->element;
        }
    }

    return nullptr;
}

void UIEditor::highlightElement(UIElement* element, Renderer& renderer) {
    SDL_Rect bounds = element->getBounds();

    renderer.setDrawColor(0, 255, 255, 255);
    renderer.drawRect(bounds.x - 1, bounds.y - 1, bounds.w + 2, bounds.h + 2);

    if (!draggingElement) {
        int handleSize = 8;

        renderer.setDrawColor(255, 255, 0, 255);
        renderer.fillRect(bounds.x - handleSize/2, bounds.y - handleSize/2, handleSize, handleSize);  // Top-left
        renderer.fillRect(bounds.x + bounds.w - handleSize/2, bounds.y - handleSize/2, handleSize, handleSize);  // Top-right
        renderer.fillRect(bounds.x - handleSize/2, bounds.y + bounds.h - handleSize/2, handleSize, handleSize);  // Bottom-left
        renderer.fillRect(bounds.x + bounds.w - handleSize/2, bounds.y + bounds.h - handleSize/2, handleSize, handleSize);  // Bottom-right

        renderer.setDrawColor(255, 200, 0, 255);
        renderer.fillRect(bounds.x - handleSize/2, bounds.y + bounds.h/2 - handleSize/2, handleSize, handleSize);  // Left
        renderer.fillRect(bounds.x + bounds.w - handleSize/2, bounds.y + bounds.h/2 - handleSize/2, handleSize, handleSize);  // Right
        renderer.fillRect(bounds.x + bounds.w/2 - handleSize/2, bounds.y - handleSize/2, handleSize, handleSize);  // Top
        renderer.fillRect(bounds.x + bounds.w/2 - handleSize/2, bounds.y + bounds.h - handleSize/2, handleSize, handleSize);  // Bottom
    }
}

void UIEditor::startDragging(UIElement* element, int mouseX, int mouseY) {
    if (!element) return;

    draggingElement = true;
    resizingElement = false;

    dragOffsetX = mouseX - element->getX();
    dragOffsetY = mouseY - element->getY();

    // For undo support, we'd store original position here
}

void UIEditor::startResizing(UIElement* element, int mouseX, int mouseY) {
    if (!element) return;

    draggingElement = false;
    resizingElement = true;

    resizeStartWidth = element->getWidth();
    resizeStartHeight = element->getHeight();
    resizeStartX = element->getX();
    resizeStartY = element->getY();

    // For undo support, we'd store original size here
}

void UIEditor::updateDragging(int mouseX, int mouseY) {
    if (!selectedElement || !draggingElement) return;

    int newX = mouseX - dragOffsetX;
    int newY = mouseY - dragOffsetY;

    // Optional: add grid snapping here

    selectedElement->setPosition(newX, newY);
}

void UIEditor::updateResizing(int mouseX, int mouseY) {
    if (!selectedElement || !resizingElement) return;

    int deltaX = mouseX - lastMouseX;
    int deltaY = mouseY - lastMouseY;

    int newX = selectedElement->getX();
    int newY = selectedElement->getY();
    int newWidth = selectedElement->getWidth();
    int newHeight = selectedElement->getHeight();

    switch (currentResizeHandle) {
        case ResizeHandle::TOP_LEFT:
            newX += deltaX;
            newY += deltaY;
            newWidth -= deltaX;
            newHeight -= deltaY;
            break;
        case ResizeHandle::TOP_RIGHT:
            newY += deltaY;
            newWidth += deltaX;
            newHeight -= deltaY;
            break;
        case ResizeHandle::BOTTOM_LEFT:
            newX += deltaX;
            newWidth -= deltaX;
            newHeight += deltaY;
            break;
        case ResizeHandle::BOTTOM_RIGHT:
            newWidth += deltaX;
            newHeight += deltaY;
            break;
        case ResizeHandle::LEFT:
            newX += deltaX;
            newWidth -= deltaX;
            break;
        case ResizeHandle::RIGHT:
            newWidth += deltaX;
            break;
        case ResizeHandle::TOP:
            newY += deltaY;
            newHeight -= deltaY;
            break;
        case ResizeHandle::BOTTOM:
            newHeight += deltaY;
            break;
        default:
            break;
    }

    if (newWidth < 10) newWidth = 10;
    if (newHeight < 10) newHeight = 10;

    selectedElement->setPosition(newX, newY);
    selectedElement->setSize(newWidth, newHeight);
}

UIEditor::ResizeHandle UIEditor::getResizeHandleAt(UIElement* element, int x, int y) {
    if (!element) return ResizeHandle::NONE;

    SDL_Rect bounds = element->getBounds();
    int handleSize = 8;

    if (x >= bounds.x - handleSize/2 && x <= bounds.x + handleSize/2 &&
        y >= bounds.y - handleSize/2 && y <= bounds.y + handleSize/2) {
        return ResizeHandle::TOP_LEFT;
    }

    if (x >= bounds.x + bounds.w - handleSize/2 && x <= bounds.x + bounds.w + handleSize/2 &&
        y >= bounds.y - handleSize/2 && y <= bounds.y + handleSize/2) {
        return ResizeHandle::TOP_RIGHT;
    }

    if (x >= bounds.x - handleSize/2 && x <= bounds.x + handleSize/2 &&
        y >= bounds.y + bounds.h - handleSize/2 && y <= bounds.y + bounds.h + handleSize/2) {
        return ResizeHandle::BOTTOM_LEFT;
    }

    if (x >= bounds.x + bounds.w - handleSize/2 && x <= bounds.x + bounds.w + handleSize/2 &&
        y >= bounds.y + bounds.h - handleSize/2 && y <= bounds.y + bounds.h + handleSize/2) {
        return ResizeHandle::BOTTOM_RIGHT;
    }

    if (x >= bounds.x - handleSize/2 && x <= bounds.x + handleSize/2 &&
        y >= bounds.y + bounds.h/2 - handleSize/2 && y <= bounds.y + bounds.h/2 + handleSize/2) {
        return ResizeHandle::LEFT;
    }

    if (x >= bounds.x + bounds.w - handleSize/2 && x <= bounds.x + bounds.w + handleSize/2 &&
        y >= bounds.y + bounds.h/2 - handleSize/2 && y <= bounds.y + bounds.h/2 + handleSize/2) {
        return ResizeHandle::RIGHT;
    }

    if (x >= bounds.x + bounds.w/2 - handleSize/2 && x <= bounds.x + bounds.w/2 + handleSize/2 &&
        y >= bounds.y - handleSize/2 && y <= bounds.y + handleSize/2) {
        return ResizeHandle::TOP;
    }

    if (x >= bounds.x + bounds.w/2 - handleSize/2 && x <= bounds.x + bounds.w/2 + handleSize/2 &&
        y >= bounds.y + bounds.h - handleSize/2 && y <= bounds.y + bounds.h + handleSize/2) {
        return ResizeHandle::BOTTOM;
    }

    return ResizeHandle::NONE;
}

bool UIEditor::saveLayout(const std::string& filename) {
    try {
        std::filesystem::create_directories("layouts");

        nlohmann::json layoutJson;
        nlohmann::json elementsJson = nlohmann::json::array();

        for (const auto& info : elementInfos) {
            UIElement* element = info.element;
            if (!element) continue;

            nlohmann::json elementJson;
            elementJson["id"] = info.id;
            elementJson["type"] = info.type;
            elementJson["x"] = element->getX();
            elementJson["y"] = element->getY();
            elementJson["width"] = element->getWidth();
            elementJson["height"] = element->getHeight();
            elementJson["visible"] = element->isVisible();
            elementJson["active"] = element->isActive();

            if (info.type == "Button") {
                UIButton* button = dynamic_cast<UIButton*>(element);
                if (button) {
                    // Would need getter methods
                    // elementJson["text"] = button->getText();
                    // elementJson["textureID"] = button->getTextureID();
                    // elementJson["hoverTextureID"] = button->getHoverTextureID();
                    // elementJson["pressedTextureID"] = button->getPressedTextureID();
                }
            }
            else if (info.type == "Label") {
                UILabel* label = dynamic_cast<UILabel*>(element);
                if (label) {
                    // Would need getter methods
                    // elementJson["text"] = label->getText();
                }
            }
            else if (info.type == "Panel") {
                UIPanel* panel = dynamic_cast<UIPanel*>(element);
                if (panel) {
                    // Would need getter methods
                    // elementJson["textureID"] = panel->getTextureID();
                }
            }

            elementsJson.push_back(elementJson);
        }

        layoutJson["elements"] = elementsJson;
        layoutJson["manager"] = (currentManager == UIManagerType::CITY) ? "city" : "arena";

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        file << layoutJson.dump(4);
        file.close();

        std::cout << "UI Layout saved to " << filename << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving layout: " << e.what() << std::endl;
        return false;
    }
}

bool UIEditor::loadLayout(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filename << std::endl;
            return false;
        }

        nlohmann::json layoutJson;
        file >> layoutJson;
        file.close();

        if (!layoutJson.contains("elements")) {
            std::cerr << "Invalid layout file format" << std::endl;
            return false;
        }

        if (layoutJson.contains("manager")) {
            std::string managerType = layoutJson["manager"];
            if (managerType == "city") {
                switchUIManager(UIManagerType::CITY);
            }
            else if (managerType == "arena") {
                switchUIManager(UIManagerType::ARENA);
            }
        }

        for (const auto& elementJson : layoutJson["elements"]) {
            std::string id = elementJson["id"];

            for (const auto& info : elementInfos) {
                if (info.id == id) {
                    UIElement* element = info.element;
                    if (!element) continue;

                    element->setPosition(elementJson["x"], elementJson["y"]);
                    element->setSize(elementJson["width"], elementJson["height"]);

                    if (elementJson.contains("visible")) {
                        element->setVisible(elementJson["visible"]);
                    }

                    if (elementJson.contains("active")) {
                        element->setActive(elementJson["active"]);
                    }

                    if (info.type == "Button") {
                        UIButton* button = dynamic_cast<UIButton*>(element);
                        if (button) {
                            if (elementJson.contains("text")) {
                                button->setText(elementJson["text"]);
                            }

                            if (elementJson.contains("textureID")) {
                                button->setTextureID(elementJson["textureID"]);
                            }

                            if (elementJson.contains("hoverTextureID")) {
                                button->setHoverTextureID(elementJson["hoverTextureID"]);
                            }

                            if (elementJson.contains("pressedTextureID")) {
                                button->setPressedTextureID(elementJson["pressedTextureID"]);
                            }
                        }
                    }
                    else if (info.type == "Label") {
                        UILabel* label = dynamic_cast<UILabel*>(element);
                        if (label && elementJson.contains("text")) {
                            label->setText(elementJson["text"]);
                        }
                    }
                    else if (info.type == "Panel") {
                        UIPanel* panel = dynamic_cast<UIPanel*>(element);
                        if (panel && elementJson.contains("textureID")) {
                            panel->setTextureID(elementJson["textureID"]);
                        }
                    }

                    break;
                }
            }
        }

        std::cout << "UI Layout loaded from " << filename << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading layout: " << e.what() << std::endl;
        return false;
    }
}

void UIEditor::refreshLayoutList() {
    availableLayouts.clear();

    try {
        std::filesystem::create_directories("layouts");

        for (const auto& entry : std::filesystem::directory_iterator("layouts")) {
            if (entry.path().extension() == ".json") {
                std::string filename = entry.path().stem().string();
                availableLayouts.push_back(filename);
            }
        }

        if (availableLayouts.empty()) {
            availableLayouts.push_back("default");
            availableLayouts.push_back("city_ui");
            availableLayouts.push_back("arena_ui");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning layouts directory: " << e.what() << std::endl;
        availableLayouts.push_back("default");
    }
}

std::string UIEditor::getLayoutPath(const std::string& layoutName) {
    return "layouts/" + layoutName + ".json";
}

void UIEditor::updateElementInfos() {
    elementInfos.clear();

    // This is a placeholder - in a real implementation we would need a way to
    // iterate through all UI elements in the UIManager and give them unique IDs

    // For example, we might have code like:
    /*
    const std::vector<UIElement*>& elements = currentUIManager->getElements();
    int counter = 0;
    for (UIElement* element : elements) {
        std::string type;
        if (dynamic_cast<UIButton*>(element)) {
            type = "Button";
        }
        else if (dynamic_cast<UILabel*>(element)) {
            type = "Label";
        }
        else if (dynamic_cast<UIPanel*>(element)) {
            type = "Panel";
        }
        else {
            type = "Element";
        }

        std::string id = type + "_" + std::to_string(counter++);
        elementInfos.push_back({id, type, element});
    }
    */

    if (currentManager == UIManagerType::CITY) {
        elementInfos.push_back({"MenuPanel", "Panel", menuPanel});
        elementInfos.push_back({"ArenaButton", "Button", arenaButton});
        elementInfos.push_back({"UpgradesButton", "Button", upgradesButton});
        elementInfos.push_back({"StatsLabel", "Label", statsLabel});
    }
    else {
        elementInfos.push_back({"BackToCityButton", "Button", cityButton});
    }
}

void UIEditor::switchUIManager(UIManagerType type) {
    currentManager = type;

    if (type == UIManagerType::CITY) {
        currentUIManager = cityUIManager;
    }
    else {
        currentUIManager = arenaUIManager;
    }

    selectedElement = nullptr;
    updateElementInfos();
}