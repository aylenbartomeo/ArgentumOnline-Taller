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
        SDL2pp::Texture& skull = textures.get(std::string(GC::RESOURCES_DIR) + SKULL_SHEET);
        const FrameRect sf = skullFrameRect();
        renderer.Copy(skull, SDL2pp::Rect(sf.x, sf.y, sf.w, sf.h),
                      SDL2pp::Rect(px - camera.x, py - camera.y, GC::TILE_SIZE, GC::TILE_SIZE));
        return;
    }

    const EntitySprite sprite = spriteForEntity(entity.type, entity.entityTypeId, entity.id);
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

    renderer.Copy(body, SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h),
                  SDL2pp::Rect(px + (GC::TILE_SIZE - bodyDstW) / 2 - camera.x,
                               py + GC::TILE_SIZE - bodyDstH - camera.y, bodyDstW, bodyDstH));

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
