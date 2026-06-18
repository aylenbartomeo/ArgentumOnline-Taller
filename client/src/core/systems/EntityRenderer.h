#ifndef ENTITY_RENDERER_H
#define ENTITY_RENDERER_H

#include <cstdint>
#include <optional>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_ttf.h>

#include "../animation/CharacterAnimator.h"
#include "../animation/CharacterSprites.h"
#include "../rendering/TextureManager.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/PlayerStatsDTO.h"
#include "common/include/dto/Snapshot.h"

class EntityRenderer {
public:
    EntityRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, uint32_t myId);

    void setFont(TTF_Font* f) { font = f; }

    // Dibuja todos los jugadores y monstruos. Purga animators obsoletos.
    void render(const CameraOffset& camera, const SnapshotDTO& snapshot, uint32_t nowMs,
                std::optional<uint32_t> selectedNpc = std::nullopt,
                const PlayerStatsDTO* localStats = nullptr);

    // Acceso a animators (necesario para FxSystem y CameraSystem).
    const std::unordered_map<uint32_t, CharacterAnimator>& getAnimators() const;
    std::unordered_map<uint32_t, CharacterAnimator>& getAnimators();

private:
    void drawEntity(const EntityDTO& entity, const CameraOffset& camera, uint32_t nowMs,
                    std::optional<uint32_t> selectedNpc, const PlayerStatsDTO* localStats);

    void drawArmorAndShield(const EntityDTO& entity, const PlayerStatsDTO* localStats, int bodyDstX,
                            int bodyDstY, int bodyDstW, int bodyDstH, int frameCol, Movement facing,
                            bool isGhostPlayer, const EntitySprite& sprite);

    void drawWeapon(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                    const CameraOffset& camera, int px, int py, int bodyDstW, int bodyDstH,
                    const FrameRect& bf, int frameCol, Movement facing, bool isGhostPlayer);

    void drawHead(const EntityDTO& entity, const CameraOffset& camera, int px, int py, int frameCol,
                  Movement facing, bool isGhostPlayer, const EntitySprite& sprite, int& outHeadX,
                  int& outHeadY);

    void drawHelmet(const EntityDTO& entity, const PlayerStatsDTO* localStats, int headX, int headY,
                    int frameCol, Movement facing, bool isGhostPlayer);

    void drawPlayerName(const EntityDTO& entity, const CameraOffset& camera, int px, int py);

    void drawHealthBar(const EntityDTO& entity, const CameraOffset& camera);

    bool isItemEquipped(const EntityDTO& entity, const PlayerStatsDTO* localStats,
                        uint32_t itemId) const;

    TextureManager& textures;
    SDL2pp::Renderer& renderer;
    uint32_t myId;
    TTF_Font* font;

    std::unordered_map<uint32_t, CharacterAnimator> animators;
};

#endif
