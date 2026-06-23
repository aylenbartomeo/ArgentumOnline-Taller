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

class EquipmentVisualRegistry;
enum class EquipSlot;

struct RenderContext {
    const EntityDTO& entity;
    const PlayerStatsDTO* localStats;
    const CameraOffset& camera;
    int px, py;
    int bodyDstX, bodyDstY;
    int bodyDstW, bodyDstH;
    FrameRect bf;
    int frameCol;
    Movement facing;
    bool isGhostPlayer;
    const EntitySprite& sprite;
    int headX = 0;
    int headY = 0;
};

class EntityRenderer {
public:
    EntityRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, uint32_t myId,
                   const EquipmentVisualRegistry& registry);

    void setFont(TTF_Font* f) { font = f; }

    void render(const CameraOffset& camera, const SnapshotDTO& snapshot, uint32_t nowMs,
                std::optional<uint32_t> selectedNpc = std::nullopt,
                const PlayerStatsDTO* localStats = nullptr);

    const std::unordered_map<uint32_t, CharacterAnimator>& getAnimators() const;
    std::unordered_map<uint32_t, CharacterAnimator>& getAnimators();

private:
    void drawEntity(const EntityDTO& entity, const CameraOffset& camera, uint32_t nowMs,
                    std::optional<uint32_t> selectedNpc, const PlayerStatsDTO* localStats);

    void drawArmor(const RenderContext& ctx);
    void drawShield(const RenderContext& ctx);
    void drawWeapon(const RenderContext& ctx);
    void drawHead(RenderContext& ctx);
    void drawHelmet(const RenderContext& ctx);

    void drawPlayerName(const EntityDTO& entity, const CameraOffset& camera, int px, int py);
    void drawHealthBar(const EntityDTO& entity, const CameraOffset& camera);

    // Obtiene el ID del item equipado para un slot específico unificando a los demás jugadores y al
    // jugador local
    std::optional<uint32_t> getEquippedItemId(const EntityDTO& entity,
                                              const PlayerStatsDTO* localStats,
                                              EquipSlot slotType) const;

    TextureManager& textures;
    SDL2pp::Renderer& renderer;
    uint32_t myId;
    TTF_Font* font;
    const EquipmentVisualRegistry& registry;

    std::unordered_map<uint32_t, CharacterAnimator> animators;
};

#endif
