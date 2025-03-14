#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_Image.h>
#include <string>
#include <map>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize(SDL_Renderer* sdlRenderer);
    void cleanup();

    void clear();
    void present();
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void fillRect(int x, int y, int w, int h);
    void fillRect(const SDL_Rect& rect);
    void drawRect(int x, int y, int w, int h);
    void drawRect(const SDL_Rect& rect);
    void drawText(const std::string& text, int x, int y);

    bool loadTexture(const std::string& id, const std::string& filePath);
    void renderTexture(const std::string& id, int x, int y, int w = 0, int h = 0,
                        SDL_Rect* clip = nullptr, double andle = 0.0,
                        SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

private:
    SDL_Renderer* renderer;
    std::map<std::string, SDL_Texture*> textureMap;
};


#endif // RENDERER_H