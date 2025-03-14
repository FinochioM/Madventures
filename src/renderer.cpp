#include "renderer.h"
#include <iostream>

Renderer::Renderer() : renderer(nullptr) {

}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initialize(SDL_Renderer* sdlRenderer) {
    renderer = sdlRenderer;
}

void Renderer::cleanup() {
    for (auto& pair : textureMap) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }

    textureMap.clear();

    renderer = nullptr;
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::fillRect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Renderer::fillRect(const SDL_Rect& rect) {
    SDL_RenderFillRect(renderer, &rect);
}

void Renderer::drawRect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}

void Renderer::drawRect(const SDL_Rect& rect) {
    SDL_RenderDrawRect(renderer, &rect);
}

void Renderer::drawText(const std::string& text, int x, int y) {
    setDrawColor(255, 255, 255, 255);
    fillRect(x, y, text.length() * 8, 15);
}

bool Renderer::loadTexture(const std::string& id, const std::string& filePath) {
    if (textureMap.find(id) != textureMap.end()) {
        return true;
    }

    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load image " << filePath << ": " << IMG_GetError() << std::endl;
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "Failed to load texture from " << filePath << ": " << SDL_GetError() << std::endl;
        return false;
    }

    textureMap[id] = texture;
    return true;
}

void Renderer::renderTexture(const std::string& id, int x, int y, int w, int h,
                            SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    auto it  = textureMap.find(id);
    if (it == textureMap.end()) {
        std::cerr << "Texture '" << id << "' not found!" << std::endl;
        return;
    }

    SDL_Rect destRect = {x, y, w, h};

    if (w == 0 || h == 0) {
        SDL_QueryTexture(it->second, NULL, NULL, &destRect.w, &destRect.h);
    }

    SDL_RenderCopyEx(renderer, it->second, clip, &destRect, angle, center, flip);
}