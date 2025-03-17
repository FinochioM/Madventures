#include <iostream>
#include <SDL2/SDL.h>

#include "game.h"
#include "renderer.h"

#ifdef main
#undef main
#endif

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_Image could not initialize! SDL_Image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("MakisAdventures",
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SCREEN_WIDTH,
                                            SCREEN_HEIGHT,
                                            SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "Window could not be created! SDL_Error " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer could not be created! SDL_Error " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    Game game;
    if (!game.initialize()) {
        std::cout << "Failed to initialize game!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    Renderer gameRenderer;
    gameRenderer.initialize(renderer);

    if (!game.loadAssets(gameRenderer)) {
        std::cout << "Failed to load game assets!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    bool quit = false;
    SDL_Event e;
    Uint32 frameStart;
    int frameTime;

    while (!quit) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            game.handleEvent(e);
        }

        game.update();

        gameRenderer.clear();
        game.render(gameRenderer);
        gameRenderer.present();

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    gameRenderer.cleanup();
    game.cleanup();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}