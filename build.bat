@echo off
g++ -o main.exe src/main.cpp src/game.cpp src/renderer.cpp src/entity.cpp src/player.cpp src/tilemap.cpp src/tile.cpp -I C:\libs\SDL2\include -L C:\libs\SDL2\lib -lSDL2main -lSDL2 -lSDL2_image -mwindows
if %errorlevel% equ 0 (
    echo Compilation successful!
) else (
    echo Compilation failed!
)