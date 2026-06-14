#include "EntityRenderer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

#include "../animation/CharacterSprites.h"
#include "../animation/Death.h"
#include "../common/GameConstants.h"
#include "../rendering/NpcVisuals.h"
#include "../ui/HealthBar.h"

namespace GC = GameConstants;

namespace {
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";
constexpr const char* SKULL_SHEET = "106.png";

}  // namespace

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
                            std::optional<uint32_t> selectedNpc) {
    for (const EntityDTO& player: snapshot.players) drawEntity(player, camera, nowMs, selectedNpc);
    for (const EntityDTO& monster: snapshot.monsters)
        drawEntity(monster, camera, nowMs, selectedNpc);

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
                                std::optional<uint32_t> selectedNpc) {
    CharacterAnimator& anim = animators[entity.id];
    anim.update(entity.x, entity.y, nowMs);
    const int px = static_cast<int>(anim.getVirtualX() * GC::TILE_SIZE);
    const int py = static_cast<int>(anim.getVirtualY() * GC::TILE_SIZE);

    if (isDead(entity.current_hp)) {
        if (entity.type == EntityType::PLAYER && isGhost(entity.stateId)) {
            // Continuar para usar el sistema de CharacterSprites con la hoja de fantasma
        } else {
            SDL2pp::Texture& skull = textures.get(std::string(GC::RESOURCES_DIR) + SKULL_SHEET);
            const FrameRect sf = skullFrameRect();
            renderer.Copy(skull, SDL2pp::Rect(sf.x, sf.y, sf.w, sf.h),
                          SDL2pp::Rect(px - camera.x, py - camera.y, GC::TILE_SIZE, GC::TILE_SIZE));
            return;
        }
    }

    const EntitySprite sprite = spriteForEntity(entity.type, entity.entityTypeId, entity.id, entity.stateId);
    SDL2pp::Texture& body = textures.get(std::string(GC::RESOURCES_DIR) + sprite.bodySheet);

    if (entity.type == EntityType::PLAYER && isGhost(entity.stateId)) {
        body.SetBlendMode(SDL_BLENDMODE_BLEND);
        body.SetAlphaMod(180);
    }

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

    renderer.Copy(body, SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h),
                  SDL2pp::Rect(px + (GC::TILE_SIZE - bodyDstW) / 2 - camera.x,
                               py + GC::TILE_SIZE - bodyDstH - camera.y, bodyDstW, bodyDstH));

    if (entity.type == EntityType::PLAYER && isGhost(entity.stateId)) {
        body.SetAlphaMod(255);
    }

    if (sprite.drawHead) {
        const FrameRect hf = headFrameRect(facing);
        SDL2pp::Texture& headSheet =
                textures.get(std::string(GC::RESOURCES_DIR) + sprite.headSheet);
        const int headX = px + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - camera.x;
        const int headY = py + GC::TILE_SIZE - GC::CHARACTER_DRAW_H + sprite.headOverlap -
                          GC::HEAD_DRAW_H - camera.y;
        renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                      SDL2pp::Rect(headX, headY, GC::HEAD_DRAW_W, GC::HEAD_DRAW_H));
    }

    // Anillo amarillo del jugador local
    if (entity.type == EntityType::PLAYER && entity.id == myId) {
        NpcVisuals::drawSelectionEllipse(renderer, px - camera.x, py - camera.y, GC::TILE_SIZE, 255,
                                         235, 0);
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
        SDL_Color color = {255, 255, 0, 255};  // Amarillo
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
