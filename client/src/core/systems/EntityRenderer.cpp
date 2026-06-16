#include "EntityRenderer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
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
}  // namespace

struct WeaponSheetInfo {
    const char* sheet;
    int yStart;
};

struct ArmorSheetInfo {
    const char* sheet;
    int yStart;
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

    const FrameRect bf = sprite.customGrid ? bodyFrameRectFor(sprite, facing, frameCol) :
                                             bodyFrameRect(facing, frameCol);

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

    renderer.Copy(body, SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h),
                  SDL2pp::Rect(bodyDstX, bodyDstY, bodyDstW, bodyDstH));

    if (isGhostPlayer)
        body.SetAlphaMod(255);


    // ── Weapon overlay (bloque original sin cambios) ──────────────────────────────
    // ── Weapon overlay ────────────────────────────────────────────────────────────
    if (entity.type == EntityType::PLAYER && entity.id == myId && localStats != nullptr &&
        !isDead(entity.current_hp)) {
        WeaponSheetInfo weaponInfo{"", 0};
        if (WeaponHelper::hasSword(*localStats))
            weaponInfo = {"items/espada.png", 0};
        else if (WeaponHelper::hasAxe(*localStats))
            weaponInfo = {"items/hacha.png", 48};
        else if (WeaponHelper::hasHammer(*localStats))
            weaponInfo = {"items/martillo.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, WeaponHelper::ARCO_SIMPLE_ID))
            weaponInfo = {"items/arco-simple.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, WeaponHelper::ARCO_COMPUESTO_ID))
            weaponInfo = {"items/arco-compuesto.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, WeaponHelper::VARA_FRESNO_WEAPON_ID))
            weaponInfo = {
                    "items/vara-fresno.png",
            };
        else if (WeaponHelper::hasEquipped(*localStats, 2022))
            weaponInfo = {"items/baculo-nudoso.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, 2023))
            weaponInfo = {"items/baculo-engarzado.png", 0};

        if (weaponInfo.sheet[0] != '\0') {
            SDL2pp::Texture& weaponTex =
                    textures.get(std::string(GC::RESOURCES_DIR) + weaponInfo.sheet);

            if (isGhostPlayer)
                weaponTex.SetAlphaMod(100);

            const int col = frameCol % WEAPON_COLS;
            const int wSrcX = col * WEAPON_CELL_W;
            const int wFrameW = (facing == Movement::RIGHT) ? WEAPON_CELL_W - 2 : WEAPON_CELL_W;
            const int wSrcY = weaponInfo.yStart + rowForFacing(facing) * WEAPON_STRIDE;
            const int wFrameH = WEAPON_STRIDE;

            const float scaleX = static_cast<float>(bodyDstW) / bf.w;
            const float scaleY = static_cast<float>(bodyDstH) / bf.h;
            const int wDstW = static_cast<int>(wFrameW * scaleX);
            const int wDstH = static_cast<int>(wFrameH * scaleY);
            const int wDstX = px + (GC::TILE_SIZE - wDstW) / 2 - camera.x -
                              (facing == Movement::RIGHT ? 8 : 0);
            const int wDstY = py + GC::TILE_SIZE - wDstH - camera.y;

            renderer.Copy(weaponTex, SDL2pp::Rect(wSrcX, wSrcY, wFrameW, wFrameH),
                          SDL2pp::Rect(wDstX, wDstY, wDstW, wDstH));

            if (isGhostPlayer)
                weaponTex.SetAlphaMod(255);
        }
    }

    // ── Armor/Shield overlay ──────────────────────────────────────────────────────
    // Mismo patrón que weapon overlay: sprites combinados (movimiento + icono al
    // final), yStart=0 salvo offset si la fila DOWN no arranca en y=0.
    if (entity.type == EntityType::PLAYER && entity.id == myId && localStats != nullptr &&
        !isDead(entity.current_hp)) {

        ArmorSheetInfo armorInfo{"", 0};
        if (WeaponHelper::hasEquipped(*localStats, 1000))
            armorInfo = {"armor/armadura-cuero.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, 1001))
            armorInfo = {"armor/armadura-placas.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, 1002))
            armorInfo = {"armor/tunica-azul.png", 0};

        ArmorSheetInfo shieldInfo{"", 0};
        if (WeaponHelper::hasEquipped(*localStats, 1020))
            shieldInfo = {"armor/escudo-tortuga.png", 0};
        else if (WeaponHelper::hasEquipped(*localStats, 1021))
            shieldInfo = {"armor/escudo-hierro.png", 0};

        auto drawArmorLayer = [&](const ArmorSheetInfo& info) {
            if (info.sheet[0] == '\0')
                return;
            SDL2pp::Texture& layerTex = textures.get(std::string(GC::RESOURCES_DIR) + info.sheet);

            if (isGhostPlayer)
                layerTex.SetAlphaMod(100);

            const int col = frameCol % WEAPON_COLS;
            const int srcX = col * WEAPON_CELL_W;
            const int srcY = info.yStart + rowForFacing(facing) * WEAPON_STRIDE;
            const int frameW = WEAPON_CELL_W;
            const int frameH = WEAPON_STRIDE;

            const float scaleX = static_cast<float>(bodyDstW) / bf.w;
            const float scaleY = static_cast<float>(bodyDstH) / bf.h;
            const int dstW = static_cast<int>(frameW * scaleX);
            const int dstH = static_cast<int>(frameH * scaleY);

            // ARMOR_OFFSET_X: corre la capa de armadura/escudo horizontalmente
            // Valor negativo = mover a la izquierda.
            constexpr int ARMOR_OFFSET_X = -3;
            const int dstX = px + (GC::TILE_SIZE - dstW) / 2 - camera.x + ARMOR_OFFSET_X;
            const int dstY = py + GC::TILE_SIZE - dstH - camera.y;

            renderer.Copy(layerTex, SDL2pp::Rect(srcX, srcY, frameW, frameH),
                          SDL2pp::Rect(dstX, dstY, dstW, dstH));

            if (isGhostPlayer)
                layerTex.SetAlphaMod(255);
        };

        // Armadura se dibuja antes que el escudo para que el escudo quede al frente.
        drawArmorLayer(armorInfo);
        drawArmorLayer(shieldInfo);
    }

    // ── Head (sin cambios) ────────────────────────────────────────────────────────
    if (sprite.drawHead) {
        const FrameRect hf = headFrameRect(facing);
        SDL2pp::Texture& headSheet =
                textures.get(std::string(GC::RESOURCES_DIR) + sprite.headSheet);
        const int headX = px + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - camera.x;
        const int headY = py + GC::TILE_SIZE - GC::CHARACTER_DRAW_H + sprite.headOverlap -
                          GC::HEAD_DRAW_H - camera.y;
        if (isGhostPlayer)
            headSheet.SetAlphaMod(100);
        renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                      SDL2pp::Rect(headX, headY, GC::HEAD_DRAW_W, GC::HEAD_DRAW_H));
        if (isGhostPlayer)
            headSheet.SetAlphaMod(255);
    }

    // Anillo amarillo del jugador local
    if (entity.type == EntityType::PLAYER && entity.id == myId) {
        NpcVisuals::drawSelectionEllipse(renderer, px - camera.x, py - camera.y, GC::TILE_SIZE, 255,
                                         235, 0);
    }

    // Dibujar nombre de los jugadores (si no están muertos)
    if (entity.type == EntityType::PLAYER && font && !entity.name.empty() &&
        !isDead(entity.current_hp)) {
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
