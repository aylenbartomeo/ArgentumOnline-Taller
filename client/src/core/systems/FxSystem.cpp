#include "FxSystem.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

#include <SDL2/SDL.h>

#include "../Targeting.h"
#include "../animation/FxAnimator.h"
#include "../common/GameConstants.h"

// Alias para las constantes globales
namespace GC = GameConstants;

// Constantes privadas (Solo le importan al FxSystem)
namespace {
constexpr const char* FX_SHEET = "19052.png";
constexpr int FX_FRAME_W = 64;
constexpr int FX_FRAME_H = 96;
constexpr int FX_COLS = 8;
constexpr int FX_FRAME_COUNT = 7;
constexpr uint32_t FX_FRAME_DUR_MS = 50;
constexpr int FX_DRAW_W = GC::TILE_SIZE * 3 / 2;
constexpr int FX_DRAW_H = FX_DRAW_W * FX_FRAME_H / FX_FRAME_W;

constexpr const char* SWORD_FX_SHEET = "2101.png";
constexpr int SWORD_FRAME_W = 32;
constexpr int SWORD_FRAME_H = 32;
constexpr int SWORD_FRAME_COLS = 5;
constexpr int SWORD_FRAME_COUNT = 21;
constexpr uint32_t SWORD_FRAME_DUR_MS = 40;
constexpr int SWORD_DRAW_W = GC::TILE_SIZE * 2;
constexpr int SWORD_DRAW_H = GC::TILE_SIZE * 2;

constexpr const char* PROJ_SHEET = "projectiles.png";
constexpr int PROJ_DRAW_W = 64;
constexpr int PROJ_DRAW_H = 64;
constexpr int PROJ_FRAME_COLS = 8;
constexpr int PROJ_FRAME_SIZE = 64;

constexpr const char* ARROW_SHEET = "2046.png";
constexpr int ARROW_FRAME_COLS = 2;
constexpr int ARROW_FRAME_W = 32;
constexpr int ARROW_FRAME_H = 32;
constexpr int ARROW_SRC_Y = 224;
constexpr int ARROW_SRC_X0 = 32;
constexpr int ARROW_DRAW_W = 48;
constexpr int ARROW_DRAW_H = 48;
constexpr uint16_t ARROW_SPRITE_ID = 200;

constexpr const char* STAFF_PROJ_SHEET = "3492.png";
constexpr int STAFF_PROJ_FRAME_W = 128;
constexpr int STAFF_PROJ_FRAME_H = 128;
constexpr int STAFF_PROJ_COLS = 4;
constexpr int STAFF_PROJ_FRAMES = 16;
constexpr uint32_t STAFF_PROJ_DUR_MS = 60;
constexpr int STAFF_PROJ_DRAW = GC::TILE_SIZE * 2;
constexpr uint16_t STAFF_SPRITE_ID = 203;

constexpr const char* EXPL_SHEET = "3471.png";
constexpr int EXPL_FRAME_W = 204;
constexpr int EXPL_FRAME_H = 204;
constexpr int EXPL_COLS = 5;
constexpr int EXPL_FRAMES = 22;
constexpr uint32_t EXPL_DUR_MS = 55;
constexpr int EXPL_DRAW = GC::TILE_SIZE * 4;

constexpr const char* NUDOSO_PROJ_SHEET = "3483.png";
constexpr int NUDOSO_PROJ_FRAME_W = 102;
constexpr int NUDOSO_PROJ_FRAME_H = 102;
constexpr int NUDOSO_PROJ_COLS = 5;
constexpr int NUDOSO_PROJ_FRAMES = 19;
constexpr uint32_t NUDOSO_PROJ_DUR_MS = 60;
constexpr int NUDOSO_PROJ_DRAW = GC::TILE_SIZE * 2;
constexpr uint16_t NUDOSO_SPRITE_ID = 202;

constexpr const char* NUDOSO_IMPACT_SHEET = "3534.png";
constexpr int NUDOSO_IMPACT_FRAME_W = 204;
constexpr int NUDOSO_IMPACT_FRAME_H = 204;
constexpr int NUDOSO_IMPACT_COLS = 5;
constexpr int NUDOSO_IMPACT_FRAMES = 25;
constexpr uint32_t NUDOSO_IMPACT_DUR_MS = 40;
constexpr int NUDOSO_IMPACT_DRAW = GC::TILE_SIZE * 3;

constexpr const char* FRESNO_IMPACT_SHEET = "3490.png";
constexpr int FRESNO_IMPACT_FRAME_W = 128;
constexpr int FRESNO_IMPACT_FRAME_H = 128;
constexpr int FRESNO_IMPACT_COLS = 4;
constexpr int FRESNO_IMPACT_FRAMES = 28;
constexpr uint32_t FRESNO_IMPACT_DUR_MS = 35;
constexpr int FRESNO_IMPACT_DRAW = GC::TILE_SIZE * 3;
constexpr uint16_t VARA_SPRITE_ID = 201;

constexpr const char* FLAUTA_HEAL_SHEET = "3456.png";
constexpr int FLAUTA_HEAL_FRAME_W = 32;
constexpr int FLAUTA_HEAL_FRAME_H = 32;
constexpr int FLAUTA_HEAL_COLS = 5;
constexpr int FLAUTA_HEAL_FRAMES = 5;
constexpr uint32_t FLAUTA_HEAL_DUR_MS = 60;
constexpr int FLAUTA_HEAL_DRAW = GC::TILE_SIZE * 2;

constexpr int BE_ATTACKED_FRAMES = 28;
constexpr uint32_t BE_ATTACKED_FRAME_DUR_MS = 40;
}  // namespace

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

FxSystem::FxDef FxSystem::defFor(FxType type) {
    switch (type) {
        case FxType::SWORD:
            return {SWORD_FX_SHEET,   SWORD_FRAME_W,     SWORD_FRAME_H,
                    SWORD_FRAME_COLS, SWORD_FRAME_COUNT, SWORD_FRAME_DUR_MS,
                    SWORD_DRAW_W,     SWORD_DRAW_H,      false};
        case FxType::EXPLOSION:
            return {EXPL_SHEET,  EXPL_FRAME_W, EXPL_FRAME_H, EXPL_COLS, EXPL_FRAMES,
                    EXPL_DUR_MS, EXPL_DRAW,    EXPL_DRAW,    false};
        case FxType::NUDOSO_IMPACT:
            return {NUDOSO_IMPACT_SHEET, NUDOSO_IMPACT_FRAME_W, NUDOSO_IMPACT_FRAME_H,
                    NUDOSO_IMPACT_COLS,  NUDOSO_IMPACT_FRAMES,  NUDOSO_IMPACT_DUR_MS,
                    NUDOSO_IMPACT_DRAW,  NUDOSO_IMPACT_DRAW,    true};
        case FxType::FRESNO_IMPACT:
            return {FRESNO_IMPACT_SHEET, FRESNO_IMPACT_FRAME_W, FRESNO_IMPACT_FRAME_H,
                    FRESNO_IMPACT_COLS,  FRESNO_IMPACT_FRAMES,  FRESNO_IMPACT_DUR_MS,
                    FRESNO_IMPACT_DRAW,  FRESNO_IMPACT_DRAW,    true};
        case FxType::FLAUTA_HEAL:
            return {FLAUTA_HEAL_SHEET, FLAUTA_HEAL_FRAME_W, FLAUTA_HEAL_FRAME_H,
                    FLAUTA_HEAL_COLS,  FLAUTA_HEAL_FRAMES,  FLAUTA_HEAL_DUR_MS,
                    FLAUTA_HEAL_DRAW,  FLAUTA_HEAL_DRAW,    true};
        default:
            return {FX_SHEET,        FX_FRAME_W, FX_FRAME_H, FX_COLS, FX_FRAME_COUNT,
                    FX_FRAME_DUR_MS, FX_DRAW_W,  FX_DRAW_H,  false};
    }
}

// ---------------------------------------------------------------------------

FxSystem::FxSystem(TextureManager& textures, SDL2pp::Renderer& renderer):
        textures(textures), renderer(renderer) {}

void FxSystem::triggerOnEntity(uint32_t targetId, uint32_t nowMs, FxType type) {
    if (type == FxType::BE_ATTACKED)
        fullscreenFx = ActiveFx{0, nowMs, 0, 0, type};
    else
        activeFx = ActiveFx{targetId, nowMs, 0, 0, type};
}

void FxSystem::triggerAtPixel(int px, int py, uint32_t nowMs, FxType type) {
    activeFx = ActiveFx{0, nowMs, px, py, type};
}

bool FxSystem::syncProjectileAnimators(uint32_t nowMs, const SnapshotDTO& snapshot) {
    bool impactOccurred = false;

    for (const ProjectileDTO& dto: snapshot.projectiles) {
        projAnimators[dto.id].update(dto, nowMs);
    }

    for (auto it = projAnimators.begin(); it != projAnimators.end();) {
        const bool alive =
                std::any_of(snapshot.projectiles.begin(), snapshot.projectiles.end(),
                            [&](const ProjectileDTO& dto) { return dto.id == it->first; });
        if (!alive) {
            const int px = static_cast<int>(it->second.getVirtualX() * GC::TILE_SIZE);
            const int py = static_cast<int>(it->second.getVirtualY() * GC::TILE_SIZE);

            FxType impactType = FxType::DEFAULT;
            if (it->second.getSpriteId() == STAFF_SPRITE_ID)
                impactType = FxType::EXPLOSION;
            else if (it->second.getSpriteId() == NUDOSO_SPRITE_ID)
                impactType = FxType::NUDOSO_IMPACT;
            else if (it->second.getSpriteId() == VARA_SPRITE_ID)
                impactType = FxType::FRESNO_IMPACT;

            activeFx = ActiveFx{0, SDL_GetTicks(), px, py, impactType};
            it = projAnimators.erase(it);

            impactOccurred = true;
        } else {
            ++it;
        }
    }
    return impactOccurred;
}

void FxSystem::render(const CameraOffset& camera, const SnapshotDTO& snapshot,
                      const std::unordered_map<uint32_t, CharacterAnimator>& animators) {
    if (!activeFx)
        return;

    const uint32_t now = SDL_GetTicks();
    const FxDef def = defFor(activeFx->type);
    const int frame = fxFrameIndex(now - activeFx->startMs, def.durMs, def.count);

    if (frame < 0) {
        activeFx.reset();
        return;
    }

    int baseX = 0;
    int baseY = 0;

    if (activeFx->targetId == 0) {
        baseX = activeFx->fixedPixelX;
        baseY = activeFx->fixedPixelY;
    } else {
        const EntityDTO* target = findEntityById(snapshot, activeFx->targetId);
        if (!target) {
            activeFx.reset();
            return;
        }
        auto ait = animators.find(activeFx->targetId);
        baseX = (ait != animators.end()) ?
                        static_cast<int>(ait->second.getVirtualX() * GC::TILE_SIZE) :
                        target->x * GC::TILE_SIZE;
        baseY = (ait != animators.end()) ?
                        static_cast<int>(ait->second.getVirtualY() * GC::TILE_SIZE) :
                        target->y * GC::TILE_SIZE;
    }

    renderOneFx(*activeFx, baseX, baseY, camera);
}

void FxSystem::renderOneFx(const ActiveFx& fx, int baseX, int baseY, const CameraOffset& camera) {
    const uint32_t now = SDL_GetTicks();
    const FxDef def = defFor(fx.type);
    const int frame = fxFrameIndex(now - fx.startMs, def.durMs, def.count);
    if (frame < 0)
        return;

    const std::string path = std::string(GC::RESOURCES_DIR) + def.sheet;
    if (!std::ifstream(path).good())
        return;

    SDL2pp::Texture& tex = textures.get(path);
    const FrameRect fr = fxFrameRect(frame, def.frameW, def.frameH, def.cols);

    int dstX, dstY;
    if (def.centeredY) {
        dstX = baseX + GC::TILE_SIZE / 2 - def.drawW / 2 - camera.x;
        dstY = baseY + GC::TILE_SIZE / 2 - def.drawH / 2 - camera.y;
    } else {
        dstX = baseX + GC::TILE_SIZE / 2 - def.drawW / 2 - camera.x;
        dstY = baseY + GC::TILE_SIZE - def.drawH - camera.y;
    }

    renderer.Copy(tex, SDL2pp::Rect(fr.x, fr.y, fr.w, fr.h),
                  SDL2pp::Rect(dstX, dstY, def.drawW, def.drawH));
}

void FxSystem::renderProjectiles(const CameraOffset& camera, uint32_t nowMs) {
    const std::string defaultPath = std::string(GC::RESOURCES_DIR) + PROJ_SHEET;
    if (!std::ifstream(defaultPath).good())
        return;
    SDL2pp::Texture& defaultSheet = textures.get(defaultPath);

    for (auto& [id, anim]: projAnimators) {
        anim.extrapolate(nowMs);

        const int px = static_cast<int>(anim.getVirtualX() * GC::TILE_SIZE) - camera.x;
        const int py = static_cast<int>(anim.getVirtualY() * GC::TILE_SIZE) - camera.y;

        anim.lastPixelX = px + camera.x;
        anim.lastPixelY = py + camera.y;

        const uint16_t spriteId = anim.getSpriteId();

        if (spriteId == ARROW_SPRITE_ID || spriteId == VARA_SPRITE_ID) {
            const std::string arrowPath = std::string(GC::RESOURCES_DIR) + ARROW_SHEET;
            if (!std::ifstream(arrowPath).good())
                continue;
            SDL2pp::Texture& arrowSheet = textures.get(arrowPath);
            const int frame = (nowMs / 80) % ARROW_FRAME_COLS;
            const int srcX = ARROW_SRC_X0 + frame * ARROW_FRAME_W;
            const SDL2pp::Rect dst(px - ARROW_DRAW_W / 2, py - ARROW_DRAW_H / 2, ARROW_DRAW_W,
                                   ARROW_DRAW_H);
            const float angle = std::atan2(anim.getVelY(), anim.getVelX()) * 180.0f / M_PI;
            renderer.Copy(arrowSheet, SDL2pp::Rect(srcX, ARROW_SRC_Y, ARROW_FRAME_W, ARROW_FRAME_H),
                          dst, angle, SDL2pp::NullOpt, SDL_FLIP_NONE);

        } else if (spriteId == STAFF_SPRITE_ID) {
            const std::string p = std::string(GC::RESOURCES_DIR) + STAFF_PROJ_SHEET;
            if (!std::ifstream(p).good())
                continue;
            SDL2pp::Texture& tex = textures.get(p);
            const int frame = (nowMs / STAFF_PROJ_DUR_MS) % STAFF_PROJ_FRAMES;
            const FrameRect fr =
                    fxFrameRect(frame, STAFF_PROJ_FRAME_W, STAFF_PROJ_FRAME_H, STAFF_PROJ_COLS);
            const SDL2pp::Rect dst(px - STAFF_PROJ_DRAW / 2, py - STAFF_PROJ_DRAW / 2,
                                   STAFF_PROJ_DRAW, STAFF_PROJ_DRAW);
            renderer.Copy(tex, SDL2pp::Rect(fr.x, fr.y, fr.w, fr.h), dst, 0.0, SDL2pp::NullOpt,
                          SDL_FLIP_NONE);

        } else if (spriteId == NUDOSO_SPRITE_ID) {
            const std::string p = std::string(GC::RESOURCES_DIR) + NUDOSO_PROJ_SHEET;
            if (!std::ifstream(p).good())
                continue;
            SDL2pp::Texture& tex = textures.get(p);
            const int frame = (nowMs / NUDOSO_PROJ_DUR_MS) % NUDOSO_PROJ_FRAMES;
            const FrameRect fr =
                    fxFrameRect(frame, NUDOSO_PROJ_FRAME_W, NUDOSO_PROJ_FRAME_H, NUDOSO_PROJ_COLS);
            const SDL2pp::Rect dst(px - NUDOSO_PROJ_DRAW / 2, py - NUDOSO_PROJ_DRAW / 2,
                                   NUDOSO_PROJ_DRAW, NUDOSO_PROJ_DRAW);
            const float angle = std::atan2(anim.getVelY(), anim.getVelX()) * 180.0f / M_PI;
            renderer.Copy(tex, SDL2pp::Rect(fr.x, fr.y, fr.w, fr.h), dst, angle, SDL2pp::NullOpt,
                          SDL_FLIP_NONE);

        } else {
            const int frame = (nowMs / 100) % 64;
            const int srcX = (frame % PROJ_FRAME_COLS) * PROJ_FRAME_SIZE;
            const int srcY = (frame / PROJ_FRAME_COLS) * PROJ_FRAME_SIZE;
            const SDL2pp::Rect dst(px - PROJ_DRAW_W / 2, py - PROJ_DRAW_H / 2, PROJ_DRAW_W,
                                   PROJ_DRAW_H);
            renderer.Copy(defaultSheet, SDL2pp::Rect(srcX, srcY, PROJ_FRAME_SIZE, PROJ_FRAME_SIZE),
                          dst, 0.0, SDL2pp::NullOpt, SDL_FLIP_NONE);
        }
    }
}

void FxSystem::renderFullscreen(int windowW, int windowH) {
    if (!fullscreenFx)
        return;

    const uint32_t elapsed = SDL_GetTicks() - fullscreenFx->startMs;
    const int frame = static_cast<int>(elapsed / BE_ATTACKED_FRAME_DUR_MS);

    if (frame >= BE_ATTACKED_FRAMES) {
        fullscreenFx.reset();
        return;
    }

    const std::string path = std::string(GC::RESOURCES_DIR) + "animation/beAttacked/" +
                             std::to_string(frame) + ".png";

    if (!std::ifstream(path).good())
        return;

    SDL2pp::Texture& tex = textures.get(path);

    const int halfFrames = BE_ATTACKED_FRAMES / 2;
    uint8_t alpha = 255;
    if (frame > halfFrames)
        alpha = static_cast<uint8_t>(255 * (BE_ATTACKED_FRAMES - frame) / halfFrames);

    tex.SetAlphaMod(alpha);
    renderer.Copy(tex, SDL2pp::NullOpt, SDL2pp::Rect(0, 0, windowW, windowH));
    tex.SetAlphaMod(255);
}

const std::unordered_map<uint32_t, ProjectileAnimator>& FxSystem::projectileAnimators() const {
    return projAnimators;
}
