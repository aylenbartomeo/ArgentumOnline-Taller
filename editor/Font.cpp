#include "Font.h"

#include <stdexcept>

Font::Font(SDL2pp::Renderer& renderer, const std::string& path, int sizePx): renderer(renderer) {
    if (!TTF_WasInit() && TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
    }
    ttfFont = TTF_OpenFont(path.c_str(), sizePx);
    if (!ttfFont) {
        throw std::runtime_error(std::string("TTF_OpenFont failed: ") + TTF_GetError());
    }
}

Font::~Font() {
    if (ttfFont) {
        TTF_CloseFont(ttfFont);
    }
}

void Font::drawString(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty()) {
        return;
    }
    SDL_Surface* sdlSurface = TTF_RenderText_Blended(ttfFont, text.c_str(), color);
    if (!sdlSurface) {
        return;
    }
    SDL2pp::Surface surface(sdlSurface);
    int w = surface.GetWidth();
    int h = surface.GetHeight();
    SDL2pp::Texture texture(renderer, surface);
    renderer.Copy(texture, SDL2pp::NullOpt, SDL2pp::Rect(x, y, w, h));
}
