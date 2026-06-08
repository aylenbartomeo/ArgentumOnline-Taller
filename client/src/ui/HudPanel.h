#ifndef HUD_PANEL_H
#define HUD_PANEL_H

#include <string>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "common/include/dto/PlayerStatsDTO.h"

#include "../rendering/TextureManager.h"

class HudPanel {
public:
    HudPanel(TextureManager& textures, const std::string& fontPath);
    ~HudPanel();

    void renderBackground(SDL2pp::Renderer& renderer);
    void render(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats);

    int slotAtPosition(int x, int y) const;

    HudPanel(const HudPanel&) = delete;
    HudPanel& operator=(const HudPanel&) = delete;

private:
    TextureManager& textures;
    TTF_Font* font;

    void drawBars(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats);
    void drawText(SDL2pp::Renderer& renderer, const std::string& text, int x, int y);
    void drawItemSprite(SDL2pp::Renderer& renderer, uint32_t itemId, int x, int y, int w, int h);
    void drawInventory(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats);
    void drawEquipment(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats);
};

#endif
