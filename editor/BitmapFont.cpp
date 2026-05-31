#include "BitmapFont.h"

BitmapFont::BitmapFont(TextureManager& textures, const std::string& sheetPath):
        textures(textures), sheetPath(sheetPath) {}

void BitmapFont::drawString(SDL2pp::Renderer& renderer, const std::string& text, int x, int y,
                            int charW, int charH) {
    SDL2pp::Texture& tex = textures.get(sheetPath);
    for (size_t i = 0; i < text.size(); ++i) {
        CharRect src = getCharRect(text[i]);
        SDL2pp::Rect srcRect(src.x, src.y, src.w, src.h);
        SDL2pp::Rect dstRect(x + static_cast<int>(i) * charW, y, charW, charH);
        renderer.Copy(tex, srcRect, dstRect);
    }
}
