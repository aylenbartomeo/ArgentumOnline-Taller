#include "HudPanel.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "common/include/OverlayRegistry.h"

#include "HudLayout.h"
#include "ItemIcons.h"

namespace {
constexpr int FONT_SIZE = 12;
constexpr const char* RESOURCES_DIR = "resources/";
constexpr const char* BACKGROUND = "resources/ventanaprincipal.png";

const OverlayDef* itemDef(uint32_t itemId) {
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    auto it = std::find_if(registry.begin(), registry.end(), [itemId](const OverlayDef& d) {
        return d.itemId != 0 && static_cast<uint32_t>(d.itemId) == itemId;
    });
    return it != registry.end() ? &(*it) : nullptr;
}

constexpr const char* HP_BAR = "resources/en_barradevida.bmp";
constexpr const char* MP_BAR = "resources/en_barrademana.bmp";
constexpr const char* XP_BAR = "resources/en_barraexperiencia.bmp";

constexpr int HP_X = 796, HP_Y = 542, HP_FULL = 200, BAR_H = 15;
constexpr int MP_X = 796, MP_Y = 581, MP_FULL = 200;
constexpr int XP_X = 796, XP_Y = 620, XP_FULL = 200;

constexpr int INV_ORIGIN_X = 783, INV_ORIGIN_Y = 172;
constexpr int INV_COLS = 5, INV_CELL = 40, INV_GAP = 2;

constexpr int EQUIP_LIST_X = 778, EQUIP_LIST_Y = 648, EQUIP_ROW_H = 20, EQUIP_ICON = 16;

constexpr int LEVEL_X = 770, LEVEL_Y = 48;
constexpr int GOLD_X = 788, GOLD_Y = 506;

const SDL_Color WHITE{255, 255, 255, 255};
}  // namespace

HudPanel::HudPanel(TextureManager& textures, const std::string& fontPath): textures(textures) {
    if (TTF_WasInit() == 0 && TTF_Init() != 0) {
        throw std::runtime_error(std::string("TTF_Init: ") + TTF_GetError());
    }
    font = TTF_OpenFont(fontPath.c_str(), FONT_SIZE);
    if (!font) {
        throw std::runtime_error(std::string("HudPanel font: ") + TTF_GetError());
    }
}

HudPanel::~HudPanel() {
    if (font) {
        TTF_CloseFont(font);
    }
}

int HudPanel::slotAtPosition(int x, int y) const {
    const int relX = x - INV_ORIGIN_X;
    const int relY = y - INV_ORIGIN_Y;
    if (relX < 0 || relY < 0) {
        return -1;
    }
    const int pitch = INV_CELL + INV_GAP;
    const int col = relX / pitch;
    const int row = relY / pitch;
    if (col >= INV_COLS || row >= 5) {
        return -1;
    }
    if (relX % pitch > INV_CELL || relY % pitch > INV_CELL) {
        return -1;
    }
    return row * INV_COLS + col;
}

bool HudPanel::isManualButtonClicked(int x, int y) const {
    // Coordenadas aproximadas del botón "MANUAL" en ventanaprincipal.png
    constexpr int MANUAL_BTN_X = 815;
    constexpr int MANUAL_BTN_Y = 10;
    constexpr int MANUAL_BTN_W = 100;
    constexpr int MANUAL_BTN_H = 25;

    return (x >= MANUAL_BTN_X && x <= MANUAL_BTN_X + MANUAL_BTN_W && y >= MANUAL_BTN_Y &&
            y <= MANUAL_BTN_Y + MANUAL_BTN_H);
}

void HudPanel::renderBackground(SDL2pp::Renderer& renderer) {
    SDL2pp::Texture& bg = textures.get(BACKGROUND);
    bg.SetBlendMode(SDL_BLENDMODE_BLEND);
    renderer.Copy(bg);
}

void HudPanel::drawBars(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats) {
    struct Bar {
        const char* sheet;
        int x;
        int y;
        int full;
        int cur;
        int max;
    };
    const Bar bars[] = {
            {HP_BAR, HP_X, HP_Y, HP_FULL, stats.currentHp, stats.maxHp},
            {MP_BAR, MP_X, MP_Y, MP_FULL, stats.currentMana, stats.maxMana},
            {XP_BAR, XP_X, XP_Y, XP_FULL, static_cast<int>(stats.expIntoLevel),
             static_cast<int>(stats.expForLevel)},
    };
    for (const Bar& b: bars) {
        const int w = barFillWidth(b.cur, b.max, b.full);
        if (w <= 0) {
            continue;
        }
        renderer.Copy(textures.get(b.sheet), SDL2pp::Rect(0, 0, w, BAR_H),
                      SDL2pp::Rect(b.x, b.y, w, BAR_H));
    }
}

void HudPanel::drawText(SDL2pp::Renderer& renderer, const std::string& text, int x, int y) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), WHITE);
    if (!surf) {
        return;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    if (!tex) {
        return;
    }
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void HudPanel::drawItemSprite(SDL2pp::Renderer& renderer, uint32_t itemId, int x, int y, int w,
                              int h) {

    if (itemId == 2010) {
        renderer.Copy(textures.get("resources/weapons/bowC.png"), SDL2pp::NullOpt,
                      SDL2pp::Rect(x, y, w, h));
        return;
    }

    const OverlayDef* def = itemDef(itemId);
    if (def == nullptr) {
        renderer.Copy(textures.get(iconForItem(itemId)), SDL2pp::NullOpt, SDL2pp::Rect(x, y, w, h));
        return;
    }
    int dw = w;
    int dh = h;
    if (def->srcW * h > def->srcH * w) {
        dh = def->srcH * w / def->srcW;
    } else {
        dw = def->srcW * h / def->srcH;
    }
    const int dx = x + (w - dw) / 2;
    const int dy = y + (h - dh) / 2;
    renderer.Copy(textures.get(std::string(RESOURCES_DIR) + def->tilesheet),
                  SDL2pp::Rect(def->srcX, def->srcY, def->srcW, def->srcH),
                  SDL2pp::Rect(dx, dy, dw, dh));
}

void HudPanel::drawSlotHighlight(SDL2pp::Renderer& renderer, int slotIndex) {
    const SlotRect r =
            inventorySlotRect(slotIndex, INV_COLS, INV_CELL, INV_GAP, INV_ORIGIN_X, INV_ORIGIN_Y);
    // Borde dorado para indicar slot seleccionado
    SDL_SetRenderDrawColor(renderer.Get(), 255, 215, 0, 255);
    SDL_Rect border{r.x - 1, r.y - 1, r.w + 2, r.h + 2};
    SDL_RenderDrawRect(renderer.Get(), &border);
    SDL_Rect borderInner{r.x - 2, r.y - 2, r.w + 4, r.h + 4};
    SDL_RenderDrawRect(renderer.Get(), &borderInner);
}

void HudPanel::drawInventory(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats) {
    if (selectedSlot >= 0) {
        drawSlotHighlight(renderer, selectedSlot);
    }
    for (const InventorySlotDTO& it: stats.inventory) {
        const SlotRect r =
                inventorySlotRect(it.slot, INV_COLS, INV_CELL, INV_GAP, INV_ORIGIN_X, INV_ORIGIN_Y);
        drawItemSprite(renderer, it.itemId, r.x, r.y, r.w, r.h);
        if (it.amount > 1) {
            drawText(renderer, std::to_string(it.amount), r.x, r.y + r.h - FONT_SIZE);
        }
    }
}

void HudPanel::drawEquipment(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats) {
    int row = 0;
    for (const InventorySlotDTO& it: stats.inventory) {
        if (!it.isEquipped) {
            continue;
        }
        const int y = EQUIP_LIST_Y + row * EQUIP_ROW_H;
        drawItemSprite(renderer, it.itemId, EQUIP_LIST_X, y, EQUIP_ICON, EQUIP_ICON);
        const OverlayDef* def = itemDef(it.itemId);
        drawText(renderer, def != nullptr ? def->name : "Equipado", EQUIP_LIST_X + EQUIP_ICON + 5,
                 y + 1);
        row++;
    }
}

void HudPanel::render(SDL2pp::Renderer& renderer, const PlayerStatsDTO& stats) {
    drawBars(renderer, stats);
    drawInventory(renderer, stats);
    drawEquipment(renderer, stats);
    drawText(renderer, "Nivel " + std::to_string(stats.level), LEVEL_X, LEVEL_Y);
    drawText(renderer, "Oro " + std::to_string(stats.gold), GOLD_X, GOLD_Y);
    drawText(renderer,
             "Exp " + std::to_string(stats.expIntoLevel) + "/" + std::to_string(stats.expForLevel),
             LEVEL_X, LEVEL_Y + 18);

    drawText(renderer, std::to_string(stats.currentHp) + "/" + std::to_string(stats.maxHp),
             HP_X + 78, HP_Y + 1);
    if (stats.maxMana > 0) {
        drawText(renderer, std::to_string(stats.currentMana) + "/" + std::to_string(stats.maxMana),
                 MP_X + 78, MP_Y + 1);
    }
}

void HudPanel::selectSlot(int slot) {
    // Toggle: si el slot ya está seleccionado, deseleccionarlo
    if (selectedSlot == slot) {
        selectedSlot = -1;
    } else {
        selectedSlot = slot;
    }
}

void HudPanel::clearSelection() { selectedSlot = -1; }

int HudPanel::getSelectedSlot() const { return selectedSlot; }
