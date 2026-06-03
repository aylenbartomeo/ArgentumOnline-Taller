#ifndef FONT_H
#define FONT_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

class Font {
private:
    SDL2pp::Renderer& renderer;
    TTF_Font* ttfFont;

public:
    Font(SDL2pp::Renderer& renderer, const std::string& path, int sizePx);
    ~Font();

    void drawString(const std::string& text, int x, int y, SDL_Color color);

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
};

#endif
