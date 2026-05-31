#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H

#include <string>

#include <SDL2pp/SDL2pp.hh>

#include "CharRect.h"
#include "TextureManager.h"

class BitmapFont {
private:
    TextureManager& textures;
    std::string sheetPath;

public:
    BitmapFont(TextureManager& textures, const std::string& sheetPath);

    void drawString(SDL2pp::Renderer& renderer, const std::string& text, int x, int y,
                    int charW, int charH);
};

#endif
