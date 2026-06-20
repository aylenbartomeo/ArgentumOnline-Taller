#include "EntityRenderer.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "../../ui/HealthBar.h"
#include "../../ui/ItemIcons.h"
#include "../animation/CharacterSprites.h"
#include "../animation/Death.h"
#include "../common/GameConstants.h"
#include "../rendering/EquipmentVisualRegistry.h"
#include "../rendering/NpcVisuals.h"

namespace GC = GameConstants;

namespace {
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";

constexpr int WEAPON_COLS = 5;
constexpr int ITEM_CELL_W = 25;
constexpr int ITEM_CELL_H = 48;
constexpr int ITEM_COLS = 5;
static constexpr int ITEM_DIR_ROW[4] = {0, 3, 1, 2};
}  // namespace

// ─── Constructor / Accesors ───────────────────────────────────────────────────

EntityRenderer::EntityRenderer(TextureManager& textures, SDL2pp::Renderer& renderer, uint32_t myId,
                               const EquipmentVisualRegistry& registry):
        textures(textures), renderer(renderer), myId(myId), font(nullptr), registry(registry) {}

const std::unordered_map<uint32_t, CharacterAnimator>& EntityRenderer::getAnimators() const {
    return animators;
}

std::unordered_map<uint32_t, CharacterAnimator>& EntityRenderer::getAnimators() {
    return animators;
}

std::optional<uint32_t> EntityRenderer::getEquippedItemId(const EntityDTO& entity,
                                                          const PlayerStatsDTO* localStats,
                                                          EquipSlot slotType) const {
    if (entity.id == myId && localStats != nullptr) {
        auto it = std::find_if(localStats->inventory.begin(), localStats->inventory.end(),
                               [slotType](const auto& item) {
                                   return item.isEquipped &&
                                          equipSlotForItem(item.itemId) == slotType;
                               });
        if (it != localStats->inventory.end()) {
            return it->itemId;
        }
        return std::nullopt;
    }

    switch (slotType) {
        case EquipSlot::WEAPON:
            return entity.weaponItemId > 0 ? std::optional<uint32_t>(entity.weaponItemId) :
                                             std::nullopt;
        case EquipSlot::ARMOR:
            return entity.bodyArmorItemId > 0 ? std::optional<uint32_t>(entity.bodyArmorItemId) :
                                                std::nullopt;
        case EquipSlot::SHIELD:
            return entity.shieldItemId > 0 ? std::optional<uint32_t>(entity.shieldItemId) :
                                             std::nullopt;
        case EquipSlot::HELMET:
            return entity.helmetItemId > 0 ? std::optional<uint32_t>(entity.helmetItemId) :
                                             std::nullopt;
        default:
            return std::nullopt;
    }
}

// ─── Render ───────────────────────────────────────────────────────────────────

void EntityRenderer::render(const CameraOffset& camera, const SnapshotDTO& snapshot, uint32_t nowMs,
                            std::optional<uint32_t> selectedNpc, const PlayerStatsDTO* localStats) {
    for (const EntityDTO& player: snapshot.players)
        drawEntity(player, camera, nowMs, selectedNpc, localStats);
    for (const EntityDTO& monster: snapshot.monsters)
        drawEntity(monster, camera, nowMs, selectedNpc, localStats);

    // Purgar animators obsoletos
    for (auto it = animators.begin(); it != animators.end();) {
        const bool inPlayers = std::any_of(snapshot.players.begin(), snapshot.players.end(),
                                           [&](const EntityDTO& p) { return p.id == it->first; });
        const bool inMonsters = std::any_of(snapshot.monsters.begin(), snapshot.monsters.end(),
                                            [&](const EntityDTO& m) { return m.id == it->first; });
        it = (inPlayers || inMonsters) ? std::next(it) : animators.erase(it);
    }

    // Precarga de la barra de vida
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
    const FrameRect bf = bodyFrameRectFor(sprite, facing, frameCol);

    const int bodyDstW = sprite.customGrid ?
                                 bf.w * sprite.bodyScale / 100 :
                                 bf.w * GC::TILE_SIZE / CHARACTER_FRAME_W * sprite.bodyScale / 100;
    const int bodyDstH = sprite.customGrid ? bf.h * sprite.bodyScale / 100 :
                                             bf.h * GC::CHARACTER_DRAW_H / CHARACTER_FRAME_H *
                                                     sprite.bodyScale / 100;

    const int bodyDstX = px + (GC::TILE_SIZE - bodyDstW) / 2 - camera.x;
    const int bodyDstY = py + GC::TILE_SIZE - bodyDstH - camera.y;

    const bool isGhostPlayer = (entity.type == EntityType::PLAYER && isGhost(entity.stateId));

    RenderContext ctx{entity,        localStats, camera,   px, py,       bodyDstX,
                      bodyDstY,      bodyDstW,   bodyDstH, bf, frameCol, facing,
                      isGhostPlayer, sprite,     0,        0};

    if (isGhostPlayer)
        body.SetAlphaMod(100);

    bool drawBaseBody = true;
    if (entity.type == EntityType::PLAYER &&
        getEquippedItemId(entity, localStats, EquipSlot::ARMOR).has_value()) {
        drawBaseBody = false;
    }

    if (drawBaseBody) {
        renderer.Copy(body, SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h),
                      SDL2pp::Rect(bodyDstX, bodyDstY, bodyDstW, bodyDstH));
    }

    if (isGhostPlayer)
        body.SetAlphaMod(255);

    // Render Layers
    drawArmor(ctx);
    drawWeapon(ctx);
    drawShield(ctx);
    drawHead(ctx);
    drawHelmet(ctx);

    // Anillo amarillo local o del NPC seleccionado
    if (entity.type == EntityType::PLAYER && entity.id == myId) {
        NpcVisuals::drawSelectionEllipse(renderer, px - camera.x, py - camera.y, GC::TILE_SIZE, 255,
                                         235, 0);
    }

    drawPlayerName(entity, camera, px, py);
}

// ─── Capas de Equipo (Estrategias) ────────────────────────────────────────────

void EntityRenderer::drawArmor(const RenderContext& ctx) {
    if (ctx.entity.type != EntityType::PLAYER || isDead(ctx.entity.current_hp))
        return;

    auto armorId = getEquippedItemId(ctx.entity, ctx.localStats, EquipSlot::ARMOR);
    if (!armorId)
        return;

    const ArmorVisualSpec* spec = registry.findArmor(*armorId);
    if (!spec)
        return;

    const RaceVisualConfig& rcfg = registry.raceConfig(ctx.entity.entityTypeId);
    std::string sheetPath = (rcfg.armor.usesCompactArmor && !spec->sheetCompact.empty()) ?
                                    spec->sheetCompact :
                                    spec->sheet;
    if (sheetPath.empty())
        return;

    SDL2pp::Texture& layerTex = textures.get(std::string(GC::RESOURCES_DIR) + sheetPath);
    if (ctx.isGhostPlayer)
        layerTex.SetAlphaMod(100);

    const int srcX = (ctx.frameCol % WEAPON_COLS) * rcfg.armor.cellW;
    const int srcY = rowForFacing(ctx.facing) * rcfg.armor.stride;
    const int srcW = rcfg.armor.cellW - rcfg.armor.trimLeft + rcfg.armor.extraRight;
    const int frameH = rcfg.armor.stride;

    const int dstW = srcW * GC::TILE_SIZE / CHARACTER_FRAME_W * ctx.sprite.bodyScale / 100;
    const int dstH = frameH * GC::CHARACTER_DRAW_H / CHARACTER_FRAME_H * ctx.sprite.bodyScale / 100;

    int offsetX = rcfg.armor.offsetX;
    int offsetY = rcfg.armor.offsetY;

    if (ctx.frameCol > 0) {
        switch (ctx.facing) {
            case Movement::DOWN:
                offsetY += rcfg.armor.walkDown;
                break;
            case Movement::UP:
                offsetY += rcfg.armor.walkUp;
                break;
            case Movement::LEFT:
                offsetX += rcfg.armor.walkLeft;
                break;
            case Movement::RIGHT:
                offsetX += rcfg.armor.walkRight;
                break;
        }
    }

    const int dstX = ctx.bodyDstX - (dstW - ctx.bodyDstW) / 2 + offsetX;
    const int dstY = ctx.bodyDstY - (dstH - ctx.bodyDstH) + offsetY;

    renderer.Copy(layerTex, SDL2pp::Rect(srcX + rcfg.armor.trimLeft, srcY, srcW, frameH),
                  SDL2pp::Rect(dstX, dstY, dstW, dstH));

    if (ctx.isGhostPlayer)
        layerTex.SetAlphaMod(255);
}

void EntityRenderer::drawShield(const RenderContext& ctx) {
    if (ctx.entity.type != EntityType::PLAYER || isDead(ctx.entity.current_hp))
        return;

    auto shieldId = getEquippedItemId(ctx.entity, ctx.localStats, EquipSlot::SHIELD);
    if (!shieldId)
        return;

    const ShieldVisualSpec* spec = registry.findShield(*shieldId);
    if (!spec)
        return;

    SDL2pp::Texture& shieldTex = textures.get(std::string(GC::RESOURCES_DIR) + spec->sheet);
    if (ctx.isGhostPlayer)
        shieldTex.SetAlphaMod(100);

    const int row = ITEM_DIR_ROW[rowForFacing(ctx.facing)];
    const int col = ctx.frameCol % ITEM_COLS;
    const SDL_Rect srcRect{col * ITEM_CELL_W, row * ITEM_CELL_H, ITEM_CELL_W, ITEM_CELL_H};

    const float scaleX = static_cast<float>(ctx.bodyDstW) / CHARACTER_FRAME_W;
    const float scaleY = static_cast<float>(ctx.bodyDstH) / CHARACTER_FRAME_H;
    const int dW = static_cast<int>(ITEM_CELL_W * scaleX);
    const int dH = static_cast<int>(ITEM_CELL_H * scaleY);

    const FacingOffset& itemOffset = spec->facingOffset[rowForFacing(ctx.facing)];
    const RaceVisualConfig& rcfg = registry.raceConfig(ctx.entity.entityTypeId);
    int raceOffsetX = rcfg.shield.offsetX;
    int raceOffsetY = rcfg.shield.offsetY;
    if (ctx.frameCol > 0) {
        switch (ctx.facing) {
            case Movement::DOWN:
                raceOffsetY += rcfg.shield.walkDown;
                break;
            case Movement::UP:
                raceOffsetY += rcfg.shield.walkUp;
                break;
            case Movement::LEFT:
                raceOffsetX += rcfg.shield.walkLeft;
                break;
            case Movement::RIGHT:
                raceOffsetX += rcfg.shield.walkRight;
                break;
        }
    }
    const SDL_Rect dstRect{ctx.bodyDstX + (ctx.bodyDstW - dW) / 2 + itemOffset.x + raceOffsetX,
                           ctx.bodyDstY + (ctx.bodyDstH - dH) / 2 + itemOffset.y + raceOffsetY, dW,
                           dH};

    const SDL_Point pivot{dW / 2, dH / 2};
    SDL_RenderCopyEx(renderer.Get(), shieldTex.Get(), &srcRect, &dstRect, 0.0, &pivot,
                     SDL_FLIP_NONE);

    if (ctx.isGhostPlayer)
        shieldTex.SetAlphaMod(255);
}

void EntityRenderer::drawWeapon(const RenderContext& ctx) {
    if (ctx.entity.type != EntityType::PLAYER || isDead(ctx.entity.current_hp))
        return;

    auto weaponId = getEquippedItemId(ctx.entity, ctx.localStats, EquipSlot::WEAPON);
    if (!weaponId)
        return;

    const WeaponAnimSpec* wcfg = registry.findWeapon(*weaponId);
    if (!wcfg)
        return;

    SDL2pp::Texture& weaponTex = textures.get(std::string(GC::RESOURCES_DIR) + wcfg->sheet);
    if (ctx.isGhostPlayer)
        weaponTex.SetAlphaMod(100);

    if (wcfg->kind == WeaponAnimKind::CUSTOM) {
        int rowIdx = rowForFacing(ctx.facing);
        const WeaponFacingFrames& facingData = wcfg->facings[rowIdx];

        int col = ctx.frameCol % facingData.frames;
        int wSrcX = static_cast<int>(col * wcfg->strideX);
        int wSrcY = static_cast<int>(facingData.row * wcfg->strideY);

        const float scaleX = static_cast<float>(ctx.bodyDstW) / ctx.bf.w;
        const float scaleY = static_cast<float>(ctx.bodyDstH) / ctx.bf.h;
        int wDstW = static_cast<int>(wcfg->cellW * scaleX);
        int wDstH = static_cast<int>(wcfg->cellH * scaleY);

        int wDstX = ctx.px + (GC::TILE_SIZE - wDstW) / 2 - ctx.camera.x -
                    (ctx.facing == Movement::RIGHT ? 8 : 0) + wcfg->offsetX;
        int wDstY = ctx.py + GC::TILE_SIZE - wDstH - ctx.camera.y + wcfg->offsetY;

        renderer.Copy(weaponTex, SDL2pp::Rect(wSrcX, wSrcY, wcfg->cellW, wcfg->cellH),
                      SDL2pp::Rect(wDstX, wDstY, wDstW, wDstH));
    } else {
        const int row = ITEM_DIR_ROW[rowForFacing(ctx.facing)];
        const int col = ctx.frameCol % ITEM_COLS;
        const SDL_Rect srcRect{col * ITEM_CELL_W, row * ITEM_CELL_H, ITEM_CELL_W, ITEM_CELL_H};

        const float scaleX = static_cast<float>(ctx.bodyDstW) / ctx.bf.w;
        const float scaleY = static_cast<float>(ctx.bodyDstH) / ctx.bf.h;
        const int wDstW = static_cast<int>(ITEM_CELL_W * scaleX);
        const int wDstH = static_cast<int>(ITEM_CELL_H * scaleY);

        int handOffsetX = 0;
        int handOffsetY = 0;
        switch (ctx.facing) {
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

        const int wDstX =
                ctx.px + (GC::TILE_SIZE - wDstW) / 2 - ctx.camera.x + handOffsetX + wcfg->offsetX;
        const int wDstY =
                ctx.py + GC::TILE_SIZE - wDstH - ctx.camera.y + handOffsetY + wcfg->offsetY;
        const SDL_Rect dstRect{wDstX, wDstY, wDstW, wDstH};

        const SDL_Point pivot{static_cast<int>(wcfg->pivotX * scaleX),
                              static_cast<int>(wcfg->pivotY * scaleY)};

        double swingAngle = 0.0;
        if (ctx.frameCol > 0) {
            const double swingPattern[5] = {0.0, 15.0, 0.0, -15.0, 0.0};
            swingAngle = swingPattern[ctx.frameCol % 5];
            if (ctx.facing == Movement::LEFT)
                swingAngle -= 10.0;
            if (ctx.facing == Movement::RIGHT)
                swingAngle += 10.0;
        } else {
            if (ctx.facing == Movement::LEFT)
                swingAngle = -15.0;
            if (ctx.facing == Movement::RIGHT)
                swingAngle = 15.0;
        }

        SDL_RenderCopyEx(renderer.Get(), weaponTex.Get(), &srcRect, &dstRect, swingAngle, &pivot,
                         SDL_FLIP_NONE);
    }

    if (ctx.isGhostPlayer)
        weaponTex.SetAlphaMod(255);
}

void EntityRenderer::drawHead(RenderContext& ctx) {
    if (!ctx.sprite.drawHead)
        return;

    static constexpr int HEAD_DIR_TO_COL[4] = {0, 3, 2, 1};
    const int headCol = HEAD_DIR_TO_COL[rowForFacing(ctx.facing)];
    FrameRect hf{ctx.sprite.headSrcX + headCol * ctx.sprite.headStrideX, ctx.sprite.headSrcY,
                 ctx.sprite.headSrcW, ctx.sprite.headSrcH};
    SDL2pp::Texture& headSheet =
            textures.get(std::string(GC::RESOURCES_DIR) + ctx.sprite.headSheet);

    const RaceVisualConfig& rcfg = registry.raceConfig(ctx.entity.entityTypeId);
    const auto& headCfg = rcfg.head;
    const auto& delta = headCfg.facingDelta[rowForFacing(ctx.facing)];

    int headOffsetX = (ctx.frameCol == 0) ? delta.idleDx : delta.moveDx;
    int headOffsetY = headCfg.extraOffsetY;

    // Cabeceo al caminar
    if (ctx.frameCol > 0) {
        switch (ctx.frameCol) {
            case 1:
            case 3:
                headOffsetY += 1;
                break;
            default:
                break;
        }
    }

    ctx.headX = ctx.px + GC::TILE_SIZE / 2 - GC::HEAD_DRAW_W / 2 - ctx.camera.x + headOffsetX;
    ctx.headY = ctx.py + GC::TILE_SIZE - GC::CHARACTER_DRAW_H + ctx.sprite.headOverlap -
                GC::HEAD_DRAW_H - ctx.camera.y + headOffsetY;

    int renderHeadX = ctx.headX;
    int renderHeadW = GC::HEAD_DRAW_W;

    if (ctx.facing == Movement::UP && (headCfg.trimSrcXOnUp > 0 || headCfg.trimRenderWOnUp > 0)) {
        hf.x += headCfg.trimSrcXOnUp;
        hf.w -= headCfg.trimSrcXOnUp;

        if (headCfg.trimRenderWOnUp > 0) {
            float scaleX = static_cast<float>(GC::HEAD_DRAW_W) / ctx.sprite.headSrcW;
            int renderTrim = static_cast<int>(headCfg.trimRenderWOnUp * scaleX);
            renderHeadX += renderTrim;
            renderHeadW -= renderTrim;
        }
    }

    if (ctx.isGhostPlayer)
        headSheet.SetAlphaMod(100);

    renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                  SDL2pp::Rect(renderHeadX, ctx.headY, renderHeadW, GC::HEAD_DRAW_H));

    if (ctx.isGhostPlayer)
        headSheet.SetAlphaMod(255);
}

void EntityRenderer::drawHelmet(const RenderContext& ctx) {
    if (ctx.entity.type != EntityType::PLAYER || isDead(ctx.entity.current_hp))
        return;

    auto helmetId = getEquippedItemId(ctx.entity, ctx.localStats, EquipSlot::HELMET);
    if (!helmetId)
        return;

    const HelmetVisualSpec* spec = registry.findHelmet(*helmetId);
    if (!spec)
        return;

    SDL2pp::Texture& helmTex = textures.get(std::string(GC::RESOURCES_DIR) + spec->sheet);
    if (ctx.isGhostPlayer)
        helmTex.SetAlphaMod(100);

    const RaceVisualConfig& rcfg = registry.raceConfig(ctx.entity.entityTypeId);

    int helmCol = 0;
    switch (ctx.facing) {
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
    }
    const int srcX = helmCol * spec->frameW;

    const float scaleX = static_cast<float>(GC::HEAD_DRAW_W) / 13.0f;
    const float scaleY = static_cast<float>(GC::HEAD_DRAW_H) / 15.0f;

    const int dstW = static_cast<int>(spec->frameW * scaleX);
    const int dstH = static_cast<int>(spec->frameH * scaleY);

    int offsetX = rcfg.helmet.offsetX;
    int offsetY = rcfg.helmet.offsetY;

    if (ctx.frameCol > 0) {
        switch (ctx.facing) {
            case Movement::DOWN:
                offsetY += rcfg.helmet.walkDown;
                break;
            case Movement::UP:
                offsetY += rcfg.helmet.walkUp;
                break;
            case Movement::LEFT:
                offsetX += rcfg.helmet.walkLeft;
                break;
            case Movement::RIGHT:
                offsetX += rcfg.helmet.walkRight;
                break;
        }
    }

    const int dstX = ctx.headX + (GC::HEAD_DRAW_W - dstW) / 2 + offsetX;
    const int dstY = ctx.headY + GC::HEAD_DRAW_H - dstH + offsetY;

    SDL_Rect srcRect{srcX, 0, spec->frameW, spec->frameH};
    SDL_Rect dstRect{dstX, dstY, dstW, dstH};

    SDL_RenderCopyEx(renderer.Get(), helmTex.Get(), &srcRect, &dstRect, 0.0, nullptr,
                     SDL_FLIP_NONE);

    if (ctx.isGhostPlayer)
        helmTex.SetAlphaMod(255);
}

// ─── Extras ───────────────────────────────────────────────────────────────────

void EntityRenderer::drawPlayerName(const EntityDTO& entity, const CameraOffset& camera, int px,
                                    int py) {
    if (entity.type != EntityType::PLAYER || !font || entity.name.empty())
        return;

    SDL_Color color =
            (entity.id == myId) ? SDL_Color{255, 255, 255, 255} : SDL_Color{255, 165, 0, 255};
    SDL_Color black = {0, 0, 0, 255};

    SDL_Surface* surfBg = TTF_RenderUTF8_Blended(font, entity.name.c_str(), black);
    if (surfBg) {
        SDL_Texture* texBg = SDL_CreateTextureFromSurface(renderer.Get(), surfBg);
        int textW = surfBg->w;
        int textH = surfBg->h;
        SDL_FreeSurface(surfBg);
        if (texBg) {
            int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
            int textY = py - camera.y + GC::TILE_SIZE + 2;
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

    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, entity.name.c_str(), color);
    if (surf) {
        SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
        int textW = surf->w;
        int textH = surf->h;
        SDL_FreeSurface(surf);
        if (textTex) {
            int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
            int textY = py - camera.y + GC::TILE_SIZE + 2;
            SDL_Rect dst{textX, textY, textW, textH};
            SDL_RenderCopy(renderer.Get(), textTex, nullptr, &dst);
            SDL_DestroyTexture(textTex);
        }
    }
}

void EntityRenderer::drawHealthBar(const EntityDTO& entity, const CameraOffset& camera) {
    CharacterAnimator& anim = animators[entity.id];
    const int px = static_cast<int>(anim.getVirtualX() * GC::TILE_SIZE);
    const int py = static_cast<int>(anim.getVirtualY() * GC::TILE_SIZE);

    if (font) {
        std::string lvlText = "LVL " + std::to_string(entity.level);
        SDL_Color color = (entity.type == EntityType::MONSTER) ?
                                  SDL_Color{255, 50, 50, 255} :
                                  ((entity.id == myId) ? SDL_Color{0, 255, 255, 255} :
                                                         SDL_Color{255, 165, 0, 255});
        SDL_Color black = {0, 0, 0, 255};

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

        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, lvlText.c_str(), color);
        if (surf) {
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
            int textW = surf->w;
            int textH = surf->h;
            SDL_FreeSurface(surf);
            if (textTex) {
                int textX = px - camera.x + (GC::TILE_SIZE - textW) / 2;
                int textY = py - camera.y - 36 - textH - 2;
                SDL_Rect dst{textX, textY, textW, textH};
                SDL_RenderCopy(renderer.Get(), textTex, nullptr, &dst);
                SDL_DestroyTexture(textTex);
            }
        }
    }

    if (isDead(entity.current_hp))
        return;

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
