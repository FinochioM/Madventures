@echo off
g++  -o main.exe src/main.cpp src/game.cpp src/renderer.cpp src/entity.cpp src/player.cpp src/tilemap.cpp src/tile.cpp src/map_editor.cpp src/combat_manager.cpp src/enemy.cpp src/ui_manager.cpp src/ui_label.cpp src/ui_button.cpp src/ui_panel.cpp src/ui_element.cpp src/ui_editor.cpp ^
        lib/imgui/imgui.cpp lib/imgui/imgui_demo.cpp lib/imgui/imgui_draw.cpp lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/backends/imgui_impl_sdl2.cpp lib/imgui/backends/imgui_impl_sdlrenderer2.cpp^
         -I C:\libs\SDL2\include -I lib -L C:\libs\SDL2\lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -mwindows
if %errorlevel% equ 0 (
    echo Compilation successful!
) else (
    echo Compilation failed!
)