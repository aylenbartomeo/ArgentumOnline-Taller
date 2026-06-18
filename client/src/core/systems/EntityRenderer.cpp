#include "EntityRenderer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "../animation/CharacterSprites.h"
#include "../animation/Death.h"
#include "../common/GameConstants.h"
#include "../common/WeaponHelper.h"
#include "../rendering/NpcVisuals.h"
#include "../ui/HealthBar.h"

namespace GC = GameConstants;

namespace {
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";
constexpr const char* SKULL_SHEET = "106.png";
constexpr int WEAPON_COLS = 5;
constexpr int WEAPON_CELL_W = 25;
constexpr int WEAPON_STRIDE = 48;

constexpr int ITEM_CELL_W = 25;
constexpr int ITEM_CELL_H = 48;
constexpr int ITEM_COLS = 5;
static constexpr int ITEM_DIR_ROW[4] = {0, 3, 1, 2};
struct RaceArmorConfig {
    int cellW = WEAPON_CELL_W;
    int stride = WEAPON_STRIDE;
    int trimLeft = 5;
    int extraRight = 2;
    int offsetX = -1;
    int offsetY = 0;
    int walkDown = -7;
    int walkUp = -8;
    int walkLeft = -5;
    int walkRight = -12;
};

template <typename Cfg>
static Cfg loadArmorConfigFile(const char* fileName) {
    Cfg cfg;
    const std::string path = std::string(GC::RESOURCES_DIR) + fileName;
    std::ifstream f(path);
    if (!f.is_open())
        std::cerr << "[ArmorConfig] NO SE PUDO ABRIR: " << path << std::endl;
    std::string key;
    int val;
    while (f >> key >> val) {
        if (key == "cell_w")
            cfg.cellW = val;
        else if (key == "stride")
            cfg.stride = val;
        else if (key == "trim_left")
            cfg.trimLeft = val;
        else if (key == "extra_right")
            cfg.extraRight = val;
        else if (key == "offset_x")
            cfg.offsetX = val;
        else if (key == "offset_y")
            cfg.offsetY = val;
        else if (key == "walk_down")
            cfg.walkDown = val;
        else if (key == "walk_up")
            cfg.walkUp = val;
        else if (key == "walk_left")
            cfg.walkLeft = val;
        else if (key == "walk_right")
            cfg.walkRight = val;
    }

    constexpr int MAX_CELL_W = 256;
    constexpr int MAX_STRIDE = 256;
    if (cfg.cellW <= 0 || cfg.cellW > MAX_CELL_W) {
        std::cerr << "[ArmorConfig] cell_w fuera de rango (" << cfg.cellW << "), clamped a default"
                  << std::endl;
        cfg.cellW = Cfg{}.cellW;
    }
    if (cfg.stride <= 0 || cfg.stride > MAX_STRIDE) {
        std::cerr << "[ArmorConfig] stride fuera de rango (" << cfg.stride << "), clamped a default"
                  << std::endl;
        cfg.stride = Cfg{}.stride;
    }
    return cfg;
}

// entityTypeId: 0 = Humano, 1 = Elfo, 2 = Enano, 3 = Gnomo
static RaceArmorConfig loadRaceArmorConfig(int entityTypeId) {
    switch (entityTypeId) {
        case 0:
            return loadArmorConfigFile<RaceArmorConfig>("armor-cfg/human_armor.cfg");
        case 1:
            return loadArmorConfigFile<RaceArmorConfig>("armor-cfg/elf_armor.cfg");
        case 2:
            return loadArmorConfigFile<RaceArmorConfig>("armor-cfg/dwarf_armor.cfg");
        case 3:
            return loadArmorConfigFile<RaceArmorConfig>(
                    "armor-cfg/gnome_armor.cfg");  // Agregamos al Gnomo
        default:
            return loadArmorConfigFile<RaceArmorConfig>("armor-cfg/human_armor.cfg");
    }
}

struct RaceHelmetConfig {
    int offsetX = 0;
    int offsetY = 0;
    int walkDown = 0;
    int walkUp = 0;
    int walkLeft = 0;
    int walkRight = 0;
};

static RaceHelmetConfig loadHelmetConfigFile(const char* fileName) {
    RaceHelmetConfig cfg;
    const std::string path = std::string(GC::RESOURCES_DIR) + fileName;
    std::ifstream f(path);
    if (!f.is_open()) {
        return cfg;
    }
    std::string key;
    int val;
    while (f >> key >> val) {
        if (key == "offset_x")
            cfg.offsetX = val;
        else if (key == "offset_y")
            cfg.offsetY = val;
        else if (key == "walk_down")
            cfg.walkDown = val;
        else if (key == "walk_up")
            cfg.walkUp = val;
        else if (key == "walk_left")
            cfg.walkLeft = val;
        else if (key == "walk_right")
            cfg.walkRight = val;
    }
    return cfg;
}

static RaceHelmetConfig loadRaceHelmetConfig(int entityTypeId) {
    switch (entityTypeId) {
        case 0:
            return loadHelmetConfigFile("helmet-cfg/human_helmet.cfg");
        case 1:
            return loadHelmetConfigFile("helmet-cfg/elf_helmet.cfg");
        case 2:
            return loadHelmetConfigFile("helmet-cfg/dwarf_helmet.cfg");
        case 3:
            return loadHelmetConfigFile("helmet-cfg/gnome_helmet.cfg");
        default:
            return loadHelmetConfigFile("helmet-cfg/human_helmet.cfg");
    }
}

}  // namespace

struct WeaponSheetInfo {
    const char* sheet;
    int yStart;
};

struct ArmorSheetInfo {
    const char* sheet;
    int yStart;
};

struct WeaponConfig {
    const char* sheet;
    int yStart;
    int pivotX;
    int pivotY;
};

// ─── Constructor / accesors ───────────────────────────────────────────────────

EntityRenderer::EntityRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, uint32_t myId):
        textures(textures), renderer(renderer), myId(myId), font(nullptr) {}

const std::unordered_map<uint32_t, CharacterAnimator>& EntityRenderer::getAnimators() const {
    return animators;
}

std::unordered_map<uint32_t, CharacterAnimator>& EntityRenderer::getAnimators() {
    return animators;
}

// ─── Render ───────────────────────────────────────────────────────────────────
void EntityRenderer::render(const CameraOffset& camera, const SnapshotDTO& snapshot, uint32_t nowMs,
                            std::optional<uint32_t> selectedNpc, const PlayerStatsDTO* localStats) {
    for (const EntityDTO& player: snapshot.players)
        drawEntity(player, camera, nowMs, selectedNpc, localStats);
    for (const EntityDTO& monster: snapshot.monsters)
        drawEntity(monster, camera, nowMs, selectedNpc, localStats);

    // Purgar animators huérfanos
    for (auto it = animators.begin(); it != animators.end();) {
        const bool inPlayers = std::any_of(snapshot.players.begin(), snapshot.players.end(),
                                           [&](const EntityDTO& p) { return p.id == it->first; });
        const bool inMonsters = std::any_of(snapshot.monsters.begin(), snapshot.monsters.end(),
                                            [&](const EntityDTO& m) { return m.id == it->first; });
        it = (inPlayers || inMonsters) ? std::next(it) : animators.erase(it);
    }

    // Precarga de la barra de vida (evita hitch en primer uso)
    (void)textures.get(std::string(GC::RESOURCES_DIR) + HEALTHBAR_SHEET);

    for (const EntityDTO& player: snapshot.players) drawHealthBar(player, camera);
    for (const EntityDTO& monster: snapshot.monsters) drawHealthBar(monster, camera);
}

static bool isGnome(const EntityDTO& entity) {
    return entity.type == EntityType::PLAYER && entity.entityTypeId == 3;
}

// ─── drawEntity ───────────────────────────────────────────────────────────────
void EntityRenderer::drawEntity(const EntityDTO& entity, const CameraOffset& camera, uint32_t nowMs,
                                std::optional<uint32_t> selectedNpc,
                                const PlayerStatsDTO* localStats) {
    CharacterAnimator& anim = animators[entity.id];
    anim.update(entity.x, entity.y, nowMs);
    const int px = static_cast<int>(anim.getVirtualX() * GC::TILE_SIZE);
    const int py = static_cast<int>(anim.getVirtualY() * GC::TILE_SIZE);

    const EntitySprite sprite =
            spriteForEntity(entity.type, entity.entityTypeId, entity.id, entity.stateId);
    SDL2pp::Texture& body = textures.get(std::string(GC::RESOURCES_DIR) + sprite.bodySheet);

    const Movement facing = anim.getFacing();
    const int frameCol = (static_cast<EntityAction>(entity.action) == EntityAction::WALKING) ?
                                 anim.frameColumn(nowMs) :
                                 0;

    const FrameRect bf = bodyFrameRectFor(sprite, facing, frameCol);

    const int bodyDstW = sprite.customGrid ?
                                 bf.w * sprite.bodyScale / 100 :
                                 bf.w * GC::TILE_SIZE / CHARACTER_FRAME_W * sprite.bodyScale / 100;

    const int bodyDstH = sprite.customGrid ? bf.h * sprite.bodyScale / 100 :
                                             bf.h * GC::CHARACTER_DRAW_H / CHARACTER_FRAME_H *
                                                     sprite.bodyScale / 100;

    const int bodyDstX = px + (GC::TILE_SIZE - bodyDstW) / 2 - camera.x;
    const int bodyDstY = py + GC::TILE_SIZE - bodyDstH - camera.y;

    // ── Ghost ─────────────────────────────────────────────────────────────────────
    const bool isGhostPlayer = (entity.type == EntityType::PLAYER && isGhost(entity.stateId));
    if (isGhostPlayer)
        body.SetAlphaMod(100);

    bool drawBaseBody = true;
    if (entity.type == EntityType::PLAYER) {
        if (isItemEquipped(entity, localStats, 1000) || isItemEquipped(entity, localStats, 1001) ||
            isItemEquipped(entity, localStats, 1002)) {
            drawBaseBody = false;
        }
    }

    if (drawBaseBody) {
        renderer.Copy(body, SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h),
                      SDL2pp::Rect(bodyDstX, bodyDstY, bodyDstW, bodyDstH));
    }

    if (isGhostPlayer)
        body.SetAlphaMod(255);

    drawArmorAndShield(entity, localStats, bodyDstX, bodyDstY, bodyDstW, bodyDstH, frameCol, facing,
                       isGhostPlayer, sprite);

    drawWeapon(entity, localStats, camera, px, py, bodyDstW, bodyDstH, bf, frameCol, facing,
               isGhostPlayer);

    int headX = 0, headY = 0;
    drawHead(entity, camera, px, py, frameCol, facing, isGhostPlayer, sprite, headX, headY);

    drawHelmet(entity, localStats, headX, headY, frameCol, facing, isGhostPlayer);

    // Anillo amarillo del jugador local
    if (entity.type == EntityType::PLAYER && entity.id == myId) {
        NpcVisuals::drawSelectionEllipse(renderer, px - camera.x, py - camera.y, GC::TILE_SIZE, 255,
                                         235, 0);
    }

    drawPlayerName(entity, camera, px, py);
}

// ─── drawHealthBar ────────────────────────────────────────────────────────────
void EntityRenderer::drawHealthBar(const EntityDTO& entity, const CameraOffset& camera) {
    if (isDead(entity.current_hp))
        return;
    CharacterAnimator& anim = animators[entity.id];
    const int px = static_cast<int>(anim.getVirtualX() * GC::TILE_SIZE);
    const int py = static_cast<int>(anim.getVirtualY() * GC::TILE_SIZE);

    if (font) {
        std::string lvlText = "LVL " + std::to_string(entity.level);
        SDL_Color color;
        if (entity.type == EntityType::MONSTER) {
            color = {255, 50, 50, 255};  // Rojo
        } else {
            color = (entity.id == myId) ?
                            SDL_Color{0, 255, 255, 255} :
                            SDL_Color{255, 165, 0, 255};  // Celeste local, Naranja otros
        }
        SDL_Color black = {0, 0, 0, 255};

        // Render black outline
        SDL_Surface* surfBg = TTF_RenderUTF8_Blended(font, lvlText.c_str(), black);
        if (surfBg) {
            SDL_Texture* texBg = SDL_CreateTextureFromSurface(renderer.Get(), surfBg);
            int textW = surfBg->w;
            int textH = surfBg->h;
            SDL_FreeSurface(surfBg);
            if (texBg) {
                int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
                int textY = py - camera.y - 36 - textH - 2;
                SDL_Rect dst1{textX - 1, textY - 1, textW, textH};
                SDL_Rect dst2{textX + 1, textY - 1, textW, textH};
                SDL_Rect dst3{textX - 1, textY + 1, textW, textH};
                SDL_Rect dst4{textX + 1, textY + 1, textW, textH};
                SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst1);
                SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst2);
                SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst3);
                SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst4);
                SDL_DestroyTexture(texBg);
            }
        }

        // Render actual text
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, lvlText.c_str(), color);
        if (surf) {
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
            int textW = surf->w;
            int textH = surf->h;
            SDL_FreeSurface(surf);
            if (textTex) {
                // 36 is BAR_OFFSET_Y from HealthBar.cpp
                int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
                int textY = py - camera.y - 36 - textH - 2;
                SDL_Rect dst{textX, textY, textW, textH};
                SDL_RenderCopy(renderer.Get(), textTex, nullptr, &dst);
                SDL_DestroyTexture(textTex);
            }
        }
    }

    const HealthBarLayout bar = computeHealthBar(entity.current_hp, entity.max_hp, px - camera.x,
                                                 py - camera.y, GC::TILE_SIZE);
    if (!bar.visible)
        return;

    SDL2pp::Texture& barSheet = textures.get(std::string(GC::RESOURCES_DIR) + HEALTHBAR_SHEET);
    const SDL2pp::Rect barSrc(0, 0, barSheet.GetWidth(), barSheet.GetHeight());

    renderer.SetDrawColor(20, 20, 20, 255);
    renderer.FillRect(
            SDL2pp::Rect(bar.background.x, bar.background.y, bar.background.w, bar.background.h));
    if (bar.fill.w > 0)
        renderer.Copy(barSheet, barSrc,
                      SDL2pp::Rect(bar.fill.x, bar.fill.y, bar.fill.w, bar.fill.h));
}

void EntityRenderer::drawArmorAndShield(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                                        int bodyDstX, int bodyDstY, int bodyDstW, int bodyDstH,
                                        int frameCol, Movement facing, bool isGhostPlayer,
                                        const EntitySprite& sprite) {
    if (entity.type != EntityType::PLAYER || isDead(entity.current_hp))
        return;

    ArmorSheetInfo armorInfo{"", 0};
    const bool useSmallArmor = (entity.entityTypeId == 2 || entity.entityTypeId == 3);

    if (isItemEquipped(entity, localStats, 1000))
        armorInfo = {useSmallArmor ? "armor/pechera-cuero-gnomo.png" : "armor/pechera-cuero.png",
                     0};
    else if (isItemEquipped(entity, localStats, 1001))
        armorInfo = {useSmallArmor ? "armor/pechera-hierro-gnomo.png" : "armor/pechera-hierro.png",
                     0};
    else if (isItemEquipped(entity, localStats, 1002))
        armorInfo = {useSmallArmor ? "armor/tunica-gnomo.png" : "armor/tunica.png", 0};

    auto drawArmorLayer = [&](const ArmorSheetInfo& info) {
        if (info.sheet[0] == '\0')
            return;
        SDL2pp::Texture& layerTex = textures.get(std::string(GC::RESOURCES_DIR) + info.sheet);

        if (isGhostPlayer)
            layerTex.SetAlphaMod(100);

        const RaceArmorConfig rcfg = loadRaceArmorConfig(entity.entityTypeId);

        const int armorCols = WEAPON_COLS;
        const int armorCellW = rcfg.cellW;
        const int armorStride = rcfg.stride;

        const int col = frameCol % armorCols;
        const int srcX = col * armorCellW;
        const int srcY = info.yStart + rowForFacing(facing) * armorStride;
        const int frameW = armorCellW;
        const int frameH = armorStride;

        const int trimLeft = rcfg.trimLeft;
        const int extraRight = rcfg.extraRight;

        const int srcW = frameW - trimLeft + extraRight;
        const int dstW = srcW * GC::TILE_SIZE / CHARACTER_FRAME_W * sprite.bodyScale / 100;
        const int dstH = frameH * GC::CHARACTER_DRAW_H / CHARACTER_FRAME_H * sprite.bodyScale / 100;

        int offsetX = rcfg.offsetX;
        int offsetY = rcfg.offsetY;

        if (frameCol > 0) {
            switch (facing) {
                case Movement::DOWN:
                    offsetY += rcfg.walkDown;
                    break;
                case Movement::UP:
                    offsetY += rcfg.walkUp;
                    break;
                case Movement::LEFT:
                    offsetX += rcfg.walkLeft;
                    break;
                case Movement::RIGHT:
                    offsetX += rcfg.walkRight;
                    break;
            }
        }

        const int dstX = bodyDstX - (dstW - bodyDstW) / 2 + offsetX;
        const int dstY = bodyDstY - (dstH - bodyDstH) + offsetY;

        renderer.Copy(layerTex, SDL2pp::Rect(srcX + trimLeft, srcY, srcW, frameH),
                      SDL2pp::Rect(dstX, dstY, dstW, dstH));

        if (isGhostPlayer)
            layerTex.SetAlphaMod(255);
    };

    drawArmorLayer(armorInfo);

    const char* shieldSheet = nullptr;
    if (isItemEquipped(entity, localStats, 1020))
        shieldSheet = "items/escudo-tortuga.png";
    else if (isItemEquipped(entity, localStats, 1021))
        shieldSheet = "items/escudo-hierro.png";

    if (shieldSheet != nullptr) {
        SDL2pp::Texture& shieldTex = textures.get(std::string(GC::RESOURCES_DIR) + shieldSheet);
        if (isGhostPlayer)
            shieldTex.SetAlphaMod(100);

        const int row = ITEM_DIR_ROW[rowForFacing(facing)];
        const int col = frameCol % ITEM_COLS;
        const SDL_Rect srcRect{col * ITEM_CELL_W, row * ITEM_CELL_H, ITEM_CELL_W, ITEM_CELL_H};

        const float scaleX = static_cast<float>(bodyDstW) / CHARACTER_FRAME_W;
        const float scaleY = static_cast<float>(bodyDstH) / CHARACTER_FRAME_H;
        const int dW = static_cast<int>(ITEM_CELL_W * scaleX);
        const int dH = static_cast<int>(ITEM_CELL_H * scaleY);

        int shieldOffsetX = 0;
        int shieldOffsetY = 2;
        switch (facing) {
            case Movement::DOWN:
                shieldOffsetX = 6;
                break;
            case Movement::UP:
                shieldOffsetX = -6;
                break;
            case Movement::LEFT:
                shieldOffsetX = -2;
                break;
            case Movement::RIGHT:
                shieldOffsetX = 2;
                break;
        }

        const SDL_Rect dstRect{bodyDstX + (bodyDstW - dW) / 2 + shieldOffsetX,
                               bodyDstY + (bodyDstH - dH) / 2 + shieldOffsetY, dW, dH};

        const SDL_Point pivot{dW / 2, dH / 2};
        SDL_RenderCopyEx(renderer.Get(), shieldTex.Get(), &srcRect, &dstRect, 0.0, &pivot,
                         SDL_FLIP_NONE);

        if (isGhostPlayer)
            shieldTex.SetAlphaMod(255);
    }
}

void EntityRenderer::drawWeapon(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                                const CameraOffset& camera, int px, int py, int bodyDstW,
                                int bodyDstH, const FrameRect& bf, int frameCol, Movement facing,
                                bool isGhostPlayer) {
    if (entity.type != EntityType::PLAYER || isDead(entity.current_hp))
        return;

    constexpr int PIV_HANDLE_X = ITEM_CELL_W / 2;
    constexpr int PIV_HANDLE_Y = ITEM_CELL_H - 12;
    constexpr int PIV_CENTER_X = ITEM_CELL_W / 2;
    constexpr int PIV_CENTER_Y = ITEM_CELL_H / 2;

    std::optional<WeaponConfig> weaponCfg;

    if (isItemEquipped(entity, localStats, WeaponHelper::SWORD_WEAPON_ID))
        weaponCfg = {"items/espada.png", 0, PIV_HANDLE_X, PIV_HANDLE_Y};
    else if (isItemEquipped(entity, localStats, WeaponHelper::HACHA_WEAPON_ID))
        weaponCfg = {"items/hacha.png", 0, PIV_HANDLE_X, PIV_HANDLE_Y};
    else if (isItemEquipped(entity, localStats, WeaponHelper::MARTILLO_WEAPON_ID))
        weaponCfg = {"items/martillo.png", 0, PIV_CENTER_X, PIV_CENTER_Y};
    else if (isItemEquipped(entity, localStats, WeaponHelper::ARCO_SIMPLE_ID))
        weaponCfg = {"items/arco-simple.png", 0, PIV_CENTER_X, PIV_CENTER_Y};
    else if (isItemEquipped(entity, localStats, WeaponHelper::ARCO_COMPUESTO_ID))
        weaponCfg = {"items/arco-compuesto.png", 0, PIV_CENTER_X, PIV_CENTER_Y};
    else if (isItemEquipped(entity, localStats, WeaponHelper::VARA_FRESNO_WEAPON_ID))
        weaponCfg = {"items/vara-fresno.png", 0, PIV_CENTER_X, PIV_CENTER_Y};
    else if (isItemEquipped(entity, localStats, 2022))
        weaponCfg = {"items/baculo-nudoso.png", 0, PIV_CENTER_X, PIV_CENTER_Y};
    else if (isItemEquipped(entity, localStats, 2023))
        weaponCfg = {"items/baculo-engarzado.png", 0, PIV_CENTER_X, PIV_CENTER_Y};

    std::string weaponSheet = "";
    if (isItemEquipped(entity, localStats, 4001))
        weaponSheet = "items/maza-de-titan-mov.png";
    else if (isItemEquipped(entity, localStats, 4002))
        weaponSheet = "items/escudo-infernal-mov.png";
    else if (isItemEquipped(entity, localStats, 4003))
        weaponSheet = "items/espada-de-magma-mov.png";
    else if (isItemEquipped(entity, localStats, 4004))
        weaponSheet = "items/arco-abismal-mov.png";

    if (!weaponSheet.empty()) {
        SDL2pp::Texture& weaponTex = textures.get(std::string(GC::RESOURCES_DIR) + weaponSheet);
        if (isGhostPlayer)
            weaponTex.SetAlphaMod(100);

        float cellCols = 5.0f;
        float cellRows = 4.0f;

        const float cellW_default = static_cast<float>(weaponTex.GetWidth()) / cellCols;
        const float cellH_default = static_cast<float>(weaponTex.GetHeight()) / cellRows;

        int frames = static_cast<int>(cellCols);
        int mappedRow = rowForFacing(facing);
        int spriteW = static_cast<int>(cellW_default);
        int spriteH = static_cast<int>(cellH_default);
        float strideX = cellW_default;
        float strideY = cellH_default;

        if (weaponSheet.find("maza-de-titan-mov.png") != std::string::npos) {
            strideX = 158.0f / 5.0f;
            strideY = 48.0f;
            spriteW = 26;
            spriteH = 46;
            switch (facing) {
                case Movement::UP:
                    mappedRow = 3;
                    frames = 1;
                    break;
                case Movement::LEFT:
                    mappedRow = 3;
                    frames = 5;
                    break;
                case Movement::RIGHT:
                    mappedRow = 2;
                    frames = 5;
                    break;
                case Movement::DOWN:
                default:
                    mappedRow = 2;
                    frames = 1;
                    break;
            }
        } else if (weaponSheet.find("espada-de-magma-mov.png") != std::string::npos) {
            strideX = 137.0f / 5.0f;
            strideY = 51.0f;
            spriteW = 24;
            spriteH = 49;
            switch (facing) {
                case Movement::UP:
                    mappedRow = 3;
                    frames = 1;
                    break;
                case Movement::LEFT:
                    mappedRow = 3;
                    frames = 5;
                    break;
                case Movement::RIGHT:
                    mappedRow = 2;
                    frames = 5;
                    break;
                case Movement::DOWN:
                default:
                    mappedRow = 2;
                    frames = 1;
                    break;
            }
        } else if (weaponSheet.find("escudo-infernal-mov.png") != std::string::npos) {
            strideX = 107.0f / 4.0f;
            strideY = 43.0f;
            spriteW = 28;
            spriteH = 43;
            switch (facing) {
                case Movement::UP:
                    mappedRow = 3;
                    frames = 4;
                    break;
                case Movement::LEFT:
                    mappedRow = 1;
                    frames = 4;
                    break;
                case Movement::RIGHT:
                    mappedRow = 0;
                    frames = 4;
                    break;
                case Movement::DOWN:
                default:
                    mappedRow = 2;
                    frames = 4;
                    break;
            }
        } else if (weaponSheet.find("arco-abismal-mov.png") != std::string::npos) {
            strideX = 130.0f / 5.0f;
            strideY = 48.0f;
            spriteW = 24;
            spriteH = 48;
            switch (facing) {
                case Movement::UP:
                    mappedRow = 1;
                    frames = 5;
                    break;
                case Movement::LEFT:
                    mappedRow = 2;
                    frames = 5;
                    break;
                case Movement::RIGHT:
                    mappedRow = 3;
                    frames = 5;
                    break;
                case Movement::DOWN:
                default:
                    mappedRow = 0;
                    frames = 5;
                    break;
            }
        }

        int col = (frameCol % frames);
        const int wSrcX = static_cast<int>(col * strideX);
        const int wSrcY = static_cast<int>(mappedRow * strideY);
        const int wFrameW = spriteW;
        const int wFrameH = spriteH;

        const float scaleX = static_cast<float>(bodyDstW) / bf.w;
        const float scaleY = static_cast<float>(bodyDstH) / bf.h;
        const int wDstW = static_cast<int>(wFrameW * scaleX);
        const int wDstH = static_cast<int>(wFrameH * scaleY);
        const int wDstX =
                px + (GC::TILE_SIZE - wDstW) / 2 - camera.x - (facing == Movement::RIGHT ? 8 : 0);
        const int wDstY = py + GC::TILE_SIZE - wDstH - camera.y;

        renderer.Copy(weaponTex, SDL2pp::Rect(wSrcX, wSrcY, wFrameW, wFrameH),
                      SDL2pp::Rect(wDstX, wDstY, wDstW, wDstH));

        if (isGhostPlayer)
            weaponTex.SetAlphaMod(255);

    } else if (weaponCfg.has_value()) {
        const WeaponConfig& wcfg = *weaponCfg;
        SDL2pp::Texture& weaponTex = textures.get(std::string(GC::RESOURCES_DIR) + wcfg.sheet);

        if (isGhostPlayer)
            weaponTex.SetAlphaMod(100);

        const int row = ITEM_DIR_ROW[rowForFacing(facing)];
        const int col = frameCol % ITEM_COLS;

        const SDL_Rect srcRect{wcfg.yStart + col * ITEM_CELL_W, row * ITEM_CELL_H, ITEM_CELL_W,
                               ITEM_CELL_H};

        const float scaleX = static_cast<float>(bodyDstW) / bf.w;
        const float scaleY = static_cast<float>(bodyDstH) / bf.h;
        const int wDstW = static_cast<int>(ITEM_CELL_W * scaleX);
        const int wDstH = static_cast<int>(ITEM_CELL_H * scaleY);

        int handOffsetX = 0;
        int handOffsetY = 0;
        switch (facing) {
            case Movement::DOWN:
                handOffsetX = -6;
                handOffsetY = 3;
                break;
            case Movement::UP:
                handOffsetX = 8;
                handOffsetY = 2;
                break;
            case Movement::LEFT:
                handOffsetX = -7;
                handOffsetY = 4;
                break;
            case Movement::RIGHT:
                handOffsetX = 2;
                handOffsetY = 4;
                break;
        }

        const int wDstX = px + (GC::TILE_SIZE - wDstW) / 2 - camera.x + handOffsetX;
        const int wDstY = py + GC::TILE_SIZE - wDstH - camera.y + handOffsetY;
        const SDL_Rect dstRect{wDstX, wDstY, wDstW, wDstH};

        const SDL_Point pivot{static_cast<int>(wcfg.pivotX * scaleX),
                              static_cast<int>(wcfg.pivotY * scaleY)};

        double swingAngle = 0.0;
        if (frameCol > 0) {
            const double swingPattern[5] = {0.0, 15.0, 0.0, -15.0, 0.0};
            swingAngle = swingPattern[frameCol % 5];
            if (facing == Movement::LEFT)
                swingAngle -= 10.0;
            if (facing == Movement::RIGHT)
                swingAngle += 10.0;
        } else {
            if (facing == Movement::LEFT)
                swingAngle = -15.0;
            if (facing == Movement::RIGHT)
                swingAngle = 15.0;
        }

        SDL_RenderCopyEx(renderer.Get(), weaponTex.Get(), &srcRect, &dstRect, swingAngle, &pivot,
                         SDL_FLIP_NONE);

        if (isGhostPlayer)
            weaponTex.SetAlphaMod(255);
    }
}

void EntityRenderer::drawHead(const EntityDTO& entity, const CameraOffset& camera, int px, int py,
                              int frameCol, Movement facing, bool isGhostPlayer,
                              const EntitySprite& sprite, int& outHeadX, int& outHeadY) {
    if (!sprite.drawHead)
        return;

    static constexpr int HEAD_DIR_TO_COL[4] = {0, 3, 2, 1};  // DOWN, UP, LEFT, RIGHT -> col
    const int headCol = HEAD_DIR_TO_COL[rowForFacing(facing)];
    FrameRect hf{sprite.headSrcX + headCol * sprite.headStrideX, sprite.headSrcY, sprite.headSrcW,
                 sprite.headSrcH};
    SDL2pp::Texture& headSheet = textures.get(std::string(GC::RESOURCES_DIR) + sprite.headSheet);

    // --- AJUSTES INDIVIDUALES DE LA CABEZA ---
    int headOffsetX = 0;
    int headOffsetY = 0;

    if (frameCol == 0) {
        switch (facing) {
            case Movement::UP:
                headOffsetX = -4;
                break;
            case Movement::RIGHT:
                headOffsetX = -3;
                break;
            case Movement::DOWN:
                headOffsetX = -2;
                break;
            case Movement::LEFT:
                headOffsetX = -3;
                break;
        }
        headOffsetY = 0;
    } else {
        // Ajustes de dirección cuando el personaje está EN MOVIMIENTO
        switch (facing) {
            case Movement::UP:
                headOffsetX = -5;
                break;
            case Movement::RIGHT:
                headOffsetX = -2;
                break;
            case Movement::DOWN:
                headOffsetX = -1;
                break;
            case Movement::LEFT:
                headOffsetX = -6;
                break;
        }

        // Animación de "cabeceo" según el paso
        switch (frameCol) {
            case 1:
                headOffsetY = 1;
                break;
            case 2:
                headOffsetY = 0;
                break;
            case 3:
                headOffsetY = 1;
                break;
            case 4:
                headOffsetY = 0;
                break;
            default:
                headOffsetY = 0;
                break;
        }
    }

    // --- Ajustes particulares para humanos (entityTypeId == 0) ---
    if (entity.type == EntityType::PLAYER && entity.entityTypeId == 0) {
        headOffsetY -= 2;  // Subir un poco la cabeza en todos los ángulos
        if (facing == Movement::UP) {
            headOffsetX -= 2;  // Apenas más a la izquierda
        }
    }

    // --- Ajustes particulares para elfos (entityTypeId == 1) ---
    if (entity.type == EntityType::PLAYER && entity.entityTypeId == 1) {
        headOffsetY -= 2;  // Subir un poco la cabeza en todos los ángulos
    }

    // --- Ajustes particulares para enanos (entityTypeId == 2) ---
    if (entity.type == EntityType::PLAYER && entity.entityTypeId == 2) {
        switch (facing) {
            case Movement::DOWN:
                headOffsetY -= 2;
                headOffsetX += 2;  // Mover 1px más a la derecha que antes
                break;
            case Movement::UP:
                headOffsetX += 5;
                // Si sale cortada es porque el recorte original en la imagen está corrido.
                // Le restamos a hf.x para que empiece a "leer" la imagen más a la izquierda
                hf.x -= 6;
                break;
            case Movement::RIGHT:
                headOffsetX += 3;  // Un pixel más a la derecha
                break;
            case Movement::LEFT:
                // Separamos quieto (frameCol == 0) de en movimiento
                if (frameCol == 0) {
                    headOffsetX += 5;  // Aún más a la derecha cuando está quieto
                } else {
                    headOffsetX += 8;  // Mucho más a la derecha cuando camina para compensar el
                                       // "vuelo"
                }
                break;
        }
    }

    // --- Ajustes particulares para gnomos (entityTypeId == 3) ---
    if (entity.type == EntityType::PLAYER && entity.entityTypeId == 3) {
        if (facing == Movement::DOWN) {
            if (frameCol == 0) {
                headOffsetX += 2;  // Quieto
            } else {
                headOffsetX +=
                        1;  // En movimiento: un poco más a la izquierda que cuando está quieto
            }
        } else if (facing == Movement::UP) {
            if (frameCol > 0) {
                headOffsetX += 1;  // En movimiento: apenas a la derecha
            }
        }
    }

    const int headX = px + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - camera.x + headOffsetX;
    const int headY = py + GC::TILE_SIZE - GC::CHARACTER_DRAW_H + sprite.headOverlap -
                      GC::HEAD_DRAW_H - camera.y + headOffsetY;

    outHeadX = headX;
    outHeadY = headY;

    int renderHeadX = headX;
    int renderHeadW = GC::HEAD_DRAW_W;

    // Se recorta la cabeza hacia arriba para Elfo (1) y Humano (0)
    // ya que a veces el frame izquierdo se "filtra" en el derecho (bleeding)
    if (entity.type == EntityType::PLAYER &&
        (entity.entityTypeId == 1 || entity.entityTypeId == 0) && facing == Movement::UP) {
        int trimLeft = 2;

        hf.x += trimLeft;
        hf.w -= trimLeft;

        float scaleX = static_cast<float>(GC::HEAD_DRAW_W) / sprite.headSrcW;
        int renderTrim = static_cast<int>(trimLeft * scaleX);

        renderHeadX += renderTrim;
        renderHeadW -= renderTrim;
    }

    if (isGhostPlayer)
        headSheet.SetAlphaMod(100);

    renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                  SDL2pp::Rect(renderHeadX, headY, renderHeadW, GC::HEAD_DRAW_H));

    if (isGhostPlayer)
        headSheet.SetAlphaMod(255);
}

void EntityRenderer::drawHelmet(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                                int headX, int headY, int frameCol, Movement facing,
                                bool isGhostPlayer) {
    if (entity.type != EntityType::PLAYER || isDead(entity.current_hp))
        return;

    const char* helmetSheet = nullptr;
    int helmFrameW = 17;
    int helmFrameH = 15;

    if (isItemEquipped(entity, localStats, 1010)) {
        helmetSheet = "armor/capucha.png";
        helmFrameH = 16;
    } else if (isItemEquipped(entity, localStats, 1011)) {
        helmetSheet = "armor/casco-hierro.png";
        helmFrameH = 15;
    } else if (isItemEquipped(entity, localStats, 1012)) {
        helmetSheet = "armor/sombrero-magico.png";
        helmFrameH = 25;
    }

    if (helmetSheet != nullptr) {
        SDL2pp::Texture& helmTex = textures.get(std::string(GC::RESOURCES_DIR) + helmetSheet);

        if (isGhostPlayer)
            helmTex.SetAlphaMod(100);

        // ─── CARGAR CONFIGURACIÓN SEGÚN LA RAZA ───
        const RaceHelmetConfig hcfg = loadRaceHelmetConfig(entity.entityTypeId);

        int helmCol = 0;
        switch (facing) {
            case Movement::DOWN:
                helmCol = 0;
                break;
            case Movement::RIGHT:
                helmCol = 1;
                break;
            case Movement::LEFT:
                helmCol = 2;
                break;
            case Movement::UP:
                helmCol = 3;
                break;
            default:
                helmCol = 0;
                break;
        }
        const int srcX = helmCol * helmFrameW;

        const float scaleX = static_cast<float>(GC::HEAD_DRAW_W) / 13.0f;
        const float scaleY = static_cast<float>(GC::HEAD_DRAW_H) / 15.0f;

        const int dstW = static_cast<int>(helmFrameW * scaleX);
        const int dstH = static_cast<int>(helmFrameH * scaleY);

        int offsetX = hcfg.offsetX;
        int offsetY = hcfg.offsetY;

        if (frameCol > 0) {
            switch (facing) {
                case Movement::DOWN:
                    offsetY += hcfg.walkDown;
                    break;
                case Movement::UP:
                    offsetY += hcfg.walkUp;
                    break;
                case Movement::LEFT:
                    offsetX += hcfg.walkLeft;
                    break;
                case Movement::RIGHT:
                    offsetX += hcfg.walkRight;
                    break;
            }
        }

        const int dstX = headX + (GC::HEAD_DRAW_W - dstW) / 2 + offsetX;
        const int dstY = headY + GC::HEAD_DRAW_H - dstH + offsetY;

        SDL_Rect srcRect{srcX, 0, helmFrameW, helmFrameH};
        SDL_Rect dstRect{dstX, dstY, dstW, dstH};

        SDL_RenderCopyEx(renderer.Get(), helmTex.Get(), &srcRect, &dstRect, 0.0, nullptr,
                         SDL_FLIP_NONE);

        if (isGhostPlayer)
            helmTex.SetAlphaMod(255);
    }
}

void EntityRenderer::drawPlayerName(const EntityDTO& entity, const CameraOffset& camera, int px,
                                    int py) {
    if (entity.type != EntityType::PLAYER || !font || entity.name.empty() ||
        isDead(entity.current_hp))
        return;

    SDL_Color color = (entity.id == myId) ?
                              SDL_Color{255, 255, 255, 255} :
                              SDL_Color{255, 165, 0, 255};  // Blanco local, Naranja otros
    SDL_Color black = {0, 0, 0, 255};

    // Render black outline
    SDL_Surface* surfBg = TTF_RenderUTF8_Blended(font, entity.name.c_str(), black);
    if (surfBg) {
        SDL_Texture* texBg = SDL_CreateTextureFromSurface(renderer.Get(), surfBg);
        int textW = surfBg->w;
        int textH = surfBg->h;
        SDL_FreeSurface(surfBg);
        if (texBg) {
            int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
            int textY = py - camera.y + GC::TILE_SIZE + 2;
            // Draw 4 offsets for outline
            SDL_Rect dst1{textX - 1, textY - 1, textW, textH};
            SDL_Rect dst2{textX + 1, textY - 1, textW, textH};
            SDL_Rect dst3{textX - 1, textY + 1, textW, textH};
            SDL_Rect dst4{textX + 1, textY + 1, textW, textH};
            SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst1);
            SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst2);
            SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst3);
            SDL_RenderCopy(renderer.Get(), texBg, nullptr, &dst4);
            SDL_DestroyTexture(texBg);
        }
    }

    // Render actual text
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, entity.name.c_str(), color);
    if (surf) {
        SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
        int textW = surf->w;
        int textH = surf->h;
        SDL_FreeSurface(surf);
        if (textTex) {
            int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
            int textY = py - camera.y + GC::TILE_SIZE + 2;  // Abajo del personaje
            SDL_Rect dst{textX, textY, textW, textH};
            SDL_RenderCopy(renderer.Get(), textTex, nullptr, &dst);
            SDL_DestroyTexture(textTex);
        }
    }
}

bool EntityRenderer::isItemEquipped(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                                    uint32_t itemId) const {
    if (entity.id == myId && localStats != nullptr) {
        return WeaponHelper::hasEquipped(*localStats, itemId);
    }
    return (entity.weaponItemId == itemId || entity.bodyArmorItemId == itemId ||
            entity.shieldItemId == itemId || entity.helmetItemId == itemId);
}
