#ifndef FX_SYSTEM_H
#define FX_SYSTEM_H

#include <optional>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>

#include "../animation/CharacterAnimator.h"
#include "../animation/ProjectileAnimator.h"
#include "../common/FxType.h"
#include "../rendering/TextureManager.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/Snapshot.h"

class FxSystem {
public:
    FxSystem(TextureManager& textures, SDL2pp::Renderer& renderer);

    // Detecta proyectiles muertos y activa FX de impacto. Llama a update() en cada animator.
    bool syncProjectileAnimators(uint32_t nowMs, const SnapshotDTO& snapshot);

    void triggerOnEntity(uint32_t targetId, uint32_t nowMs, FxType type);
    void triggerAtPixel(int px, int py, uint32_t nowMs, FxType type);

    void render(const CameraOffset& camera, const SnapshotDTO& snapshot,
                const std::unordered_map<uint32_t, CharacterAnimator>& animators);

    void renderProjectiles(const CameraOffset& camera, uint32_t nowMs);

    const std::unordered_map<uint32_t, ProjectileAnimator>& projectileAnimators() const;

private:
    struct FxDef {
        const char* sheet;
        int frameW, frameH, cols, count;
        uint32_t durMs;
        int drawW, drawH;
        bool centeredY;  // true = y centrado, false = alineado al suelo
    };

    static FxDef defFor(FxType type);

    void renderOneFx(const ActiveFx& fx, int baseX, int baseY, const CameraOffset& camera);

    TextureManager& textures;
    SDL2pp::Renderer& renderer;

    std::optional<ActiveFx> activeFx;
    std::unordered_map<uint32_t, ProjectileAnimator> projAnimators;
};

#endif
