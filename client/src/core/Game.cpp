#include "Game.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "../animation/CharacterSprites.h"
#include "../animation/Death.h"
#include "../animation/FxAnimator.h"
#include "../ui/GroundItemLabel.h"
#include "../ui/HealthBar.h"
#include "common/include/dto/CheatDTO.h"
#include "common/include/dto/ClientCommands.h"
#include "common/include/dto/StartMoveDTO.h"

#include "OverlayRegistry.h"
#include "Targeting.h"

namespace {
constexpr int TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr int VIEW_X = 0;
constexpr int VIEW_Y = 29;
constexpr int VIEW_W = 757;
constexpr int VIEW_H = 736;
constexpr const char* HUD_FONT_PATH = "resources/fonts/DejaVuSans.ttf";
constexpr Uint32 MOVE_INTERVAL_MS = 200;

constexpr const char* CHAT_FONT_PATH = "resources/fonts/DejaVuSans.ttf";

constexpr const char* RESOURCES_DIR = "resources/";
constexpr int CHARACTER_FRAME_X = 2;
constexpr int CHARACTER_FRAME_Y = 4;
constexpr int CHARACTER_FRAME_W = 24;
constexpr int CHARACTER_FRAME_H = 44;
constexpr int CHARACTER_DRAW_H = TILE_SIZE * 3 / 2;
constexpr double TAU = 6.283185307179586;
constexpr int MARKER_SEGMENTS = 24;
constexpr int MARKER_SHIFT_X = 3;

constexpr const char* HEAD_SHEET = "420.png";
constexpr int HEAD_DRAW_W = 18;
constexpr int HEAD_DRAW_H = 20;

constexpr const char* HEALTHBAR_SHEET = "en_barradevida.bmp";

constexpr const char* SKULL_SHEET = "106.png";

constexpr const char* FX_SHEET = "19052.png";
constexpr int FX_FRAME_W = 64;
constexpr int FX_FRAME_H = 96;
constexpr int FX_COLS = 8;
constexpr int FX_FRAME_COUNT = 7;
constexpr uint32_t FX_FRAME_DUR_MS = 50;
constexpr int FX_DRAW_W = TILE_SIZE * 3 / 2;
constexpr int FX_DRAW_H = FX_DRAW_W * FX_FRAME_H / FX_FRAME_W;
constexpr int ATTACK_RANGE_TILES = 1;

constexpr const char* GROUND_SHEET = "5108.png";
constexpr int GROUND_SRC_X = 416;
constexpr int GROUND_SRC_Y = 384;
constexpr int DARK_GROUND_SRC_X = 512;
constexpr int DARK_GROUND_SRC_Y = 480;
constexpr int GROUND_TILE = 32;

constexpr int PROJ_DRAW_W = 64;
constexpr int PROJ_DRAW_H = 64;
constexpr int PROJ_FRAME_COLS = 8;
constexpr int PROJ_FRAME_SIZE = 64;
constexpr const char* PROJ_SHEET = "projectiles.png";

constexpr const char* ARROW_SHEET = "2046.png";
constexpr int ARROW_FRAME_COLS = 2;
constexpr int ARROW_FRAME_ROWS = 1;
constexpr int ARROW_FRAME_W = 32;
constexpr int ARROW_FRAME_H = 32;
constexpr int ARROW_SRC_Y = 224;
constexpr int ARROW_SRC_X0 = 32;
constexpr int ARROW_DRAW_W = 48;
constexpr int ARROW_DRAW_H = 48;
constexpr uint16_t ARROW_SPRITE_ID = 200;

const char* citizenSheet(const std::string& type) {
    if (type == "merchant")
        return "1077.png";
    if (type == "banker")
        return "1071.png";
    if (type == "priest")
        return "1910.png";
    return "1200.png";
}

constexpr int CITIZEN_HEAD_OVERLAP = 6;

FrameRect citizenHead(const std::string& type) {
    if (type == "merchant")
        return FrameRect{115, 13, 13, 15};
    if (type == "banker")
        return FrameRect{142, 13, 13, 15};
    if (type == "priest")
        return FrameRect{170, 13, 11, 15};
    return FrameRect{6, 13, 13, 15};
}

std::string readWholeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("No pude abrir el archivo: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

Game::Game(Client& client):
        sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
        window("Argentum Online - Client", WINDOW_WIDTH, WINDOW_HEIGHT),
        events(),
        client(client),
        textures(window.getRenderer()),
        map(readWholeFile("maps/defaultMap.json")),
        miniChat(CHAT_FONT_PATH),
        hud(textures, HUD_FONT_PATH),
        chatParser(),
        lastSnapshot(),
        lastMoveSentMs(0) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window.getRenderer().SetLogicalSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    worldFont = TTF_OpenFont(HUD_FONT_PATH, 12);
    if (worldFont == nullptr) {
        std::cerr << "No pude abrir la fuente del texto del mundo: " << TTF_GetError() << std::endl;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "[AUDIO] Mix_OpenAudio error: " << Mix_GetError() << std::endl;
    } else {
        bgMusic = Mix_LoadMUS("resources/audio/music/game_theme.mp3");
        if (!bgMusic) {
            std::cerr << "[AUDIO] No se pudo cargar la música: " << Mix_GetError() << std::endl;
        } else {
            Mix_PlayMusic(bgMusic, -1);
            Mix_VolumeMusic(64);
        }
    }
}

Game::~Game() {
    if (worldFont != nullptr) {
        TTF_CloseFont(worldFont);
    }
    if (bgMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgMusic);
    }
    Mix_CloseAudio();
    Mix_Quit();
}

void Game::run() {
    while (true) {
        const FrameInput input = events.pollEvents();
        if (input.quit) {
            break;
        }
        miniChat.update(input, VIEW_W, VIEW_H);
        drainIncomingChat();
        processChatInput(input);
        processCheats(input);
        processEquipInput(input);
        sendMoveIfDue(input);
        render(input);
        SDL_Delay(16);
    }
}

void Game::processEquipInput(const FrameInput& input) {
    if (!input.equipPressed) {
        return;
    }
    float logicalX = 0.0f, logicalY = 0.0f;
    SDL_RenderWindowToLogical(window.getRenderer().Get(), input.equipX, input.equipY, &logicalX,
                              &logicalY);
    const int slot = hud.slotAtPosition(input.equipX, input.equipY);
    if (slot >= 0) {
        client.sendCommand(EquipItemDTO{static_cast<uint8_t>(slot)});
    }
}

void Game::processCheats(const FrameInput& input) {
    if (input.cheatLevelUp) {
        client.sendCommand(CheatDTO{CheatType::LEVEL_UP});
    }
    if (input.cheatDie) {
        client.sendCommand(CheatDTO{CheatType::DIE});
    }
    if (input.cheatGiveBow) {
        client.sendCommand(CheatDTO{CheatType::GIVE_BOW});
    }
    if (input.cheatInfiniteMana) {
        client.sendCommand(CheatDTO{CheatType::INFINITE_MANA});
    }
    if (input.cheatGiveGold) {
        client.sendCommand(CheatDTO{CheatType::GIVE_GOLD});
    }
}

void Game::drainIncomingChat() {
    ChatDTO chat;
    while (client.tryPopChatMessage(chat)) {
        miniChat.pushMessage(chat.message);
    }
}

void Game::processChatInput(const FrameInput& input) {
    if (!input.chatSubmitted || input.chatText.empty())
        return;
    std::optional<CommandVariant> cmdOpt = chatParser.parse(input.chatText);

    // Si parse devolvió un valor (has_value), lo mandamos al servidor
    if (cmdOpt.has_value()) {
        client.sendCommand(cmdOpt.value());
    } else {
        miniChat.pushMessage("[Info] Comando inexistente o mal formateado.");
    }
}

void Game::sendMoveIfDue(const FrameInput& input) {
    if (input.chatInputActive)
        return;

    const EntityDTO* localPlayer = findEntityById(lastSnapshot, client.getClientId());
    if (localPlayer != nullptr && isDead(localPlayer->current_hp)) {
        return;
    }

    const Uint32 now = SDL_GetTicks();
    if (now - lastMoveSentMs < MOVE_INTERVAL_MS) {
        return;
    }

    std::optional<Movement> direction;
    if (input.moveNorth)
        direction = Movement::UP;
    else if (input.moveSouth)
        direction = Movement::DOWN;
    else if (input.moveEast)
        direction = Movement::RIGHT;
    else if (input.moveWest)
        direction = Movement::LEFT;

    if (direction) {
        client.sendCommand(StartMoveDTO(*direction));
        lastMoveSentMs = now;
    }
}

void Game::processCombatInput(const FrameInput& input, const CameraOffset& camera) {
    if (input.resurrectPressed) {
        client.sendCommand(ResurrectDTO{});
    }

    const EntityDTO* localPlayer = findEntityById(lastSnapshot, client.getClientId());

    if (localPlayer != nullptr && isDead(localPlayer->current_hp))
        return;

    if (input.shootPressed) {
        float logicalX = 0.f, logicalY = 0.f;
        SDL_RenderWindowToLogical(window.getRenderer().Get(), input.shootScreenX,
                                  input.shootScreenY, &logicalX, &logicalY);
        const Cell cell = screenToCell(static_cast<int>(logicalX), static_cast<int>(logicalY),
                                       camera.x, camera.y, TILE_SIZE);
        client.sendCommand(ShootDTO{static_cast<float>(cell.col), static_cast<float>(cell.row)});
    }

    if (!input.attackPressed)
        return;

    float logicalX = 0.0f, logicalY = 0.0f;
    SDL_RenderWindowToLogical(window.getRenderer().Get(), input.attackX, input.attackY, &logicalX,
                              &logicalY);
    const int mouseX = static_cast<int>(logicalX);
    const int mouseY = static_cast<int>(logicalY);

    if (miniChat.isMouseOver(mouseX, mouseY, WINDOW_HEIGHT))
        return;

    const Cell cell = screenToCell(mouseX, mouseY, camera.x, camera.y, TILE_SIZE);
    const std::optional<uint32_t> target = pickTargetAt(cell.col, cell.row, lastSnapshot,
                                                        client.getClientId(), ATTACK_RANGE_TILES);
    if (target) {
        client.sendCommand(AttackDTO{*target});
        activeFx = ActiveFx{*target, SDL_GetTicks()};
    }
}

void Game::renderFx(const CameraOffset& camera) {
    if (!activeFx) {
        return;
    }

    const uint32_t now = SDL_GetTicks();
    const int frame = fxFrameIndex(now - activeFx->startMs, FX_FRAME_DUR_MS, FX_FRAME_COUNT);
    if (frame < 0) {
        activeFx.reset();
        return;
    }

    int baseX = 0;
    int baseY = 0;

    // --- RAMIFICA LA OBTENCIÓN DE COORDENADAS ---
    if (activeFx->targetId == 0) {
        // 1. Caso FX estático (ej: impacto de un proyectil).
        // Usamos la posición en píxeles que guardamos al momento de su muerte.
        baseX = activeFx->fixedPixelX;
        baseY = activeFx->fixedPixelY;

    } else {
        // 2. Caso FX sobre una entidad viva (ej: meditación o daño a un jugador).
        const EntityDTO* target = findEntityById(lastSnapshot, activeFx->targetId);

        // Si la entidad murió y desapareció del snapshot mientras el FX seguía activo, lo
        // cancelamos.
        if (!target) {
            activeFx.reset();
            return;
        }

        auto ait = animators.find(activeFx->targetId);
        baseX = (ait != animators.end()) ? static_cast<int>(ait->second.getVirtualX() * TILE_SIZE) :
                                           target->x * TILE_SIZE;
        baseY = (ait != animators.end()) ? static_cast<int>(ait->second.getVirtualY() * TILE_SIZE) :
                                           target->y * TILE_SIZE;
    }

    const std::string path = std::string(RESOURCES_DIR) + FX_SHEET;
    if (!std::ifstream(path).good()) {
        return;
    }

    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& fx = textures.get(path);
    const FrameRect fr = fxFrameRect(frame, FX_FRAME_W, FX_FRAME_H, FX_COLS);

    const int dstX = baseX + TILE_SIZE / 2 - FX_DRAW_W / 2 - camera.x;
    const int dstY = baseY + TILE_SIZE - FX_DRAW_H - camera.y;
    const SDL2pp::Rect dst(dstX, dstY, FX_DRAW_W, FX_DRAW_H);

    renderer.Copy(fx, SDL2pp::Rect(fr.x, fr.y, fr.w, fr.h), dst);
}

void Game::syncProjectileAnimators(uint32_t nowMs) {
    // Actualizar o crear animators para proyectiles del snapshot
    for (const ProjectileDTO& dto: lastSnapshot.projectiles) {
        projectileAnimators[dto.id].update(dto, nowMs);
    }

    // Detectar proyectiles que desaparecieron → activar FX de impacto
    for (auto it = projectileAnimators.begin(); it != projectileAnimators.end();) {
        const bool stillAlive =
                std::any_of(lastSnapshot.projectiles.begin(), lastSnapshot.projectiles.end(),
                            [&](const ProjectileDTO& dto) { return dto.id == it->first; });

        if (!stillAlive) {
            // Interpolar la última posición del proyectil en el momento de su muerte
            const int px = static_cast<int>(it->second.getVirtualX() * TILE_SIZE);
            const int py = static_cast<int>(it->second.getVirtualY() * TILE_SIZE);

            // Setear el FX estático (targetId = 0 indica que no sigue a nadie)
            activeFx = ActiveFx{0, SDL_GetTicks(), px, py};

            it = projectileAnimators.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::renderProjectiles(const CameraOffset& camera) {
    const uint32_t now = SDL_GetTicks();
    SDL2pp::Renderer& renderer = window.getRenderer();

    const std::string defaultPath = std::string(RESOURCES_DIR) + PROJ_SHEET;
    if (!std::ifstream(defaultPath).good())
        return;

    SDL2pp::Texture& defaultSheet = textures.get(defaultPath);

    for (auto& [id, anim]: projectileAnimators) {
        anim.extrapolate(now);

        const int px = static_cast<int>(anim.getVirtualX() * TILE_SIZE) - camera.x;
        const int py = static_cast<int>(anim.getVirtualY() * TILE_SIZE) - camera.y;

        anim.lastPixelX = px + camera.x;
        anim.lastPixelY = py + camera.y;
        printf("[PROJ] id=%u spriteId=%u\n", id, anim.getSpriteId());
        if (anim.getSpriteId() == ARROW_SPRITE_ID) {
            const std::string arrowPath = std::string(RESOURCES_DIR) + ARROW_SHEET;
            if (!std::ifstream(arrowPath).good())
                continue;

            SDL2pp::Texture& arrowSheet = textures.get(arrowPath);

            const int frame = (now / 80) % ARROW_FRAME_COLS;
            const int srcX = ARROW_SRC_X0 + frame * ARROW_FRAME_W;
            const int srcY = ARROW_SRC_Y;

            const SDL2pp::Rect dst(px - ARROW_DRAW_W / 2, py - ARROW_DRAW_H / 2, ARROW_DRAW_W,
                                   ARROW_DRAW_H);

            // Rotar la flecha según dirección de vuelo
            const float angle = std::atan2(anim.getVelY(), anim.getVelX()) * 180.0f / M_PI;
            renderer.Copy(arrowSheet, SDL2pp::Rect(srcX, srcY, ARROW_FRAME_W, ARROW_FRAME_H), dst,
                          angle, SDL2pp::NullOpt, SDL_FLIP_NONE);
        } else {
            // Comportamiento original: fuego
            const int frame = (now / 100) % 64;
            const int srcX = (frame % PROJ_FRAME_COLS) * PROJ_FRAME_SIZE;
            const int srcY = (frame / PROJ_FRAME_COLS) * PROJ_FRAME_SIZE;
            const SDL2pp::Rect dst(px - PROJ_DRAW_W / 2, py - PROJ_DRAW_H / 2, PROJ_DRAW_W,
                                   PROJ_DRAW_H);

            renderer.Copy(defaultSheet, SDL2pp::Rect(srcX, srcY, PROJ_FRAME_SIZE, PROJ_FRAME_SIZE),
                          dst, 0.0, SDL2pp::NullOpt, SDL_FLIP_NONE);
        }
    }
}

void Game::render(const FrameInput& input) {
    SnapshotDTO incoming;
    while (client.tryPopSnapshot(incoming)) {
        lastSnapshot = incoming;
    }
    PlayerStatsDTO incomingStats;
    while (client.tryPopPlayerStats(incomingStats)) {
        lastStats = incomingStats;
    }

    SDL2pp::Renderer& renderer = window.getRenderer();
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    const CameraOffset camera = computeCamera();
    processCombatInput(input, camera);

    const uint32_t now = SDL_GetTicks();
    syncProjectileAnimators(now);

    renderTerrain(camera);
    renderOverlays(camera);
    renderGroundItems(camera);
    renderCitizens(camera);
    renderEntities(camera);
    renderProjectiles(camera);
    renderFx(camera);

    // MiniChat superpuesto
    miniChat.render(renderer.Get(), VIEW_X + VIEW_W, VIEW_Y + VIEW_H, input.chatInputActive,
                    input.chatText);

    hud.renderBackground(renderer);
    hud.render(renderer, lastStats);
    renderer.Present();
}

CameraOffset Game::computeCamera() {
    const uint32_t myId = client.getClientId();
    int focusX = 0, focusY = 0;
    auto ait = animators.find(myId);
    if (ait != animators.end()) {
        focusX = static_cast<int>(ait->second.getVirtualX() * TILE_SIZE) + TILE_SIZE / 2;
        focusY = static_cast<int>(ait->second.getVirtualY() * TILE_SIZE) + TILE_SIZE / 2;
    } else {
        auto it = std::find_if(lastSnapshot.players.begin(), lastSnapshot.players.end(),
                               [myId](const EntityDTO& entity) { return entity.id == myId; });
        if (it != lastSnapshot.players.end()) {
            focusX = it->x * TILE_SIZE + TILE_SIZE / 2;
            focusY = it->y * TILE_SIZE + TILE_SIZE / 2;
        }
    }

    CameraOffset cam = computeCameraOffset(focusX, focusY, VIEW_W, VIEW_H,
                                           map.getWidth() * TILE_SIZE, map.getHeight() * TILE_SIZE);
    cam.x -= VIEW_X;
    cam.y -= VIEW_Y;
    return cam;
}

void Game::renderTerrain(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& ground = textures.get(std::string(RESOURCES_DIR) + GROUND_SHEET);
    const SDL2pp::Rect groundSrc(GROUND_SRC_X, GROUND_SRC_Y, GROUND_TILE, GROUND_TILE);
    const SDL2pp::Rect darkGroundSrc(DARK_GROUND_SRC_X, DARK_GROUND_SRC_Y, GROUND_TILE,
                                     GROUND_TILE);

    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            const SDL2pp::Rect dstRect(col * TILE_SIZE - camera.x, row * TILE_SIZE - camera.y,
                                       TILE_SIZE, TILE_SIZE);
            renderer.Copy(ground, cellInSafeZone(col, row) ? darkGroundSrc : groundSrc, dstRect);
        }
    }
    renderer.SetClipRect();
}

void Game::renderGroundItems(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    const std::vector<OverlayDef>& registry = getOverlayRegistry();

    for (const auto& item: lastSnapshot.groundItems) {
        auto it = std::find_if(registry.begin(), registry.end(), [&item](const OverlayDef& def) {
            return static_cast<uint32_t>(def.itemId) == item.itemId;
        });

        if (it != registry.end()) {
            const OverlayDef& def = *it;
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
            const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
            const int dstW = TILE_SIZE;
            const int dstH = (def.srcH * TILE_SIZE) / def.srcW;
            const int dstX = item.x * TILE_SIZE - camera.x;
            const int dstY = item.y * TILE_SIZE + TILE_SIZE - dstH - camera.y;

            renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));

            if (auto label = groundAmountLabel(item.amount)) {
                drawGroundAmount(renderer, *label, item.x, item.y, camera);
            }
        }
    }
}

void Game::drawGroundAmount(SDL2pp::Renderer& renderer, const std::string& text, int tileX,
                            int tileY, const CameraOffset& camera) {
    if (worldFont == nullptr) {
        return;
    }
    const SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(worldFont, text.c_str(), white);
    if (surf == nullptr) {
        return;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer.Get(), surf);
    const int w = surf->w;
    const int h = surf->h;
    SDL_FreeSurface(surf);
    if (tex == nullptr) {
        return;
    }
    const int x = tileX * TILE_SIZE + TILE_SIZE / 2 - w / 2 - camera.x;
    const int y = tileY * TILE_SIZE + TILE_SIZE - h - camera.y;
    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_Rect shadow{x + 1, y + 1, w, h};
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &shadow);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_Rect dst{x, y, w, h};
    SDL_RenderCopy(renderer.Get(), tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void Game::renderCitizens(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& headSheet = textures.get(std::string(RESOURCES_DIR) + HEAD_SHEET);
    const SDL2pp::Rect srcRect(CHARACTER_FRAME_X, CHARACTER_FRAME_Y, CHARACTER_FRAME_W,
                               CHARACTER_FRAME_H);
    for (const MapCitizen& citizen: map.getCitizens()) {
        SDL2pp::Texture& body =
                textures.get(std::string(RESOURCES_DIR) + citizenSheet(citizen.type));
        const SDL2pp::Rect dstRect(citizen.x * TILE_SIZE - camera.x,
                                   citizen.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H - camera.y,
                                   TILE_SIZE, CHARACTER_DRAW_H);
        renderer.Copy(body, srcRect, dstRect);

        const FrameRect hf = citizenHead(citizen.type);
        const int headX = citizen.x * TILE_SIZE + TILE_SIZE / 2 - HEAD_DRAW_W / 2 - camera.x;
        const int headY = citizen.y * TILE_SIZE + TILE_SIZE - CHARACTER_DRAW_H +
                          CITIZEN_HEAD_OVERLAP - HEAD_DRAW_H - camera.y;
        renderer.Copy(headSheet, SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h),
                      SDL2pp::Rect(headX, headY, HEAD_DRAW_W, HEAD_DRAW_H));
    }
}

bool Game::cellInSafeZone(int col, int row) const {
    return std::any_of(map.getSafeZones().begin(), map.getSafeZones().end(),
                       [col, row](const SafeZoneRect& zone) {
                           return col >= zone.x && col < zone.x + zone.width && row >= zone.y &&
                                  row < zone.y + zone.height;
                       });
}

void Game::renderOverlays(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    const std::vector<OverlayDef>& registry = getOverlayRegistry();
    for (int row = 0; row < map.getHeight(); ++row) {
        for (int col = 0; col < map.getWidth(); ++col) {
            int tileId = map.tileAt(col, row);
            if (tileId <= 0 || tileId > static_cast<int>(registry.size())) {
                continue;
            }
            const OverlayDef& def = registry[tileId - 1];
            if (!def.solid) {
                continue;
            }
            SDL2pp::Texture& tex = textures.get(std::string(RESOURCES_DIR) + def.tilesheet);
            const SDL2pp::Rect srcRect(def.srcX, def.srcY, def.srcW, def.srcH);
            const int dstW = TILE_SIZE;
            const int dstH = (def.srcH * TILE_SIZE) / def.srcW;
            const int dstX = col * TILE_SIZE - camera.x;
            const int dstY = row * TILE_SIZE + TILE_SIZE - dstH - camera.y;
            renderer.Copy(tex, srcRect, SDL2pp::Rect(dstX, dstY, dstW, dstH));
        }
    }
}

void Game::renderEntities(const CameraOffset& camera) {
    SDL2pp::Renderer& renderer = window.getRenderer();
    SDL2pp::Texture& barSheet = textures.get(std::string(RESOURCES_DIR) + HEALTHBAR_SHEET);
    const uint32_t myId = client.getClientId();
    const uint32_t now = SDL_GetTicks();

    auto drawEntity = [&](const EntityDTO& entity) {
        CharacterAnimator& anim = animators[entity.id];
        anim.update(entity.x, entity.y, now);
        const int px = static_cast<int>(anim.getVirtualX() * TILE_SIZE);
        const int py = static_cast<int>(anim.getVirtualY() * TILE_SIZE);

        if (isDead(entity.current_hp)) {
            SDL2pp::Texture& skull = textures.get(std::string(RESOURCES_DIR) + SKULL_SHEET);
            const FrameRect sf = skullFrameRect();
            const SDL2pp::Rect skullDst(px - camera.x, py - camera.y, TILE_SIZE, TILE_SIZE);
            renderer.Copy(skull, SDL2pp::Rect(sf.x, sf.y, sf.w, sf.h), skullDst);
            return;
        }
        const EntitySprite sprite = spriteForEntity(entity.type, entity.sprite_id);
        SDL2pp::Texture& body = textures.get(std::string(RESOURCES_DIR) + sprite.bodySheet);

        int bodyW = sprite.bodySrcW;
        int bodyH = sprite.bodySrcH;
        SDL2pp::Rect bodySrc(sprite.bodySrcX, sprite.bodySrcY, bodyW, bodyH);
        SDL2pp::Rect headSrcRect(sprite.headSrcX, sprite.headSrcY, sprite.headSrcW,
                                 sprite.headSrcH);
        if (entity.type == EntityType::PLAYER) {
            const Movement facing = anim.getFacing();
            const FrameRect bf = bodyFrameRect(facing, anim.frameColumn(now));
            bodySrc = SDL2pp::Rect(bf.x, bf.y, bf.w, bf.h);
            bodyW = bf.w;
            bodyH = bf.h;
            const FrameRect hf = headFrameRect(facing);
            headSrcRect = SDL2pp::Rect(hf.x, hf.y, hf.w, hf.h);
        }

        const int bodyDstW = bodyW * TILE_SIZE / CHARACTER_FRAME_W * sprite.bodyScale / 100;
        const int bodyDstH = bodyH * CHARACTER_DRAW_H / CHARACTER_FRAME_H * sprite.bodyScale / 100;
        const SDL2pp::Rect dstRect(px + (TILE_SIZE - bodyDstW) / 2 - camera.x,
                                   py + TILE_SIZE - bodyDstH - camera.y, bodyDstW, bodyDstH);
        renderer.Copy(body, bodySrc, dstRect);

        if (sprite.drawHead) {
            SDL2pp::Texture& headSheet =
                    textures.get(std::string(RESOURCES_DIR) + sprite.headSheet);
            const int headX = px + TILE_SIZE / 2 - HEAD_DRAW_W / 2 - camera.x;
            const int headY =
                    py + TILE_SIZE - CHARACTER_DRAW_H + sprite.headOverlap - HEAD_DRAW_H - camera.y;
            renderer.Copy(headSheet, headSrcRect,
                          SDL2pp::Rect(headX, headY, HEAD_DRAW_W, HEAD_DRAW_H));
        }

        if (entity.type == EntityType::PLAYER && entity.id == myId) {
            renderer.SetDrawColor(255, 235, 0, 255);
            const int cx = px + TILE_SIZE / 2 - MARKER_SHIFT_X - camera.x;
            const int cy = py + TILE_SIZE - 4 - camera.y;
            for (int t = -1; t <= 1; ++t) {
                const int rx = TILE_SIZE / 2 - 2 + t;
                const int ry = TILE_SIZE / 5 + t;
                for (int i = 0; i < MARKER_SEGMENTS; ++i) {
                    const double a0 = TAU * i / MARKER_SEGMENTS;
                    const double a1 = TAU * (i + 1) / MARKER_SEGMENTS;
                    renderer.DrawLine(cx + static_cast<int>(rx * std::cos(a0)),
                                      cy + static_cast<int>(ry * std::sin(a0)),
                                      cx + static_cast<int>(rx * std::cos(a1)),
                                      cy + static_cast<int>(ry * std::sin(a1)));
                }
            }
        }
    };

    for (const EntityDTO& player: lastSnapshot.players) {
        drawEntity(player);
    }
    for (const EntityDTO& monster: lastSnapshot.monsters) {
        drawEntity(monster);
    }

    for (auto it = animators.begin(); it != animators.end();) {
        const bool inPlayers = std::any_of(lastSnapshot.players.begin(), lastSnapshot.players.end(),
                                           [&](const EntityDTO& p) { return p.id == it->first; });
        const bool inMonsters =
                std::any_of(lastSnapshot.monsters.begin(), lastSnapshot.monsters.end(),
                            [&](const EntityDTO& m) { return m.id == it->first; });
        if (inPlayers || inMonsters) {
            ++it;
        } else {
            it = animators.erase(it);
        }
    }

    const SDL2pp::Rect barSrc(0, 0, barSheet.GetWidth(), barSheet.GetHeight());
    auto drawHealthBar = [&](const EntityDTO& entity) {
        if (isDead(entity.current_hp)) {
            return;
        }
        CharacterAnimator& anim = animators[entity.id];
        const int px = static_cast<int>(anim.getVirtualX() * TILE_SIZE);
        const int py = static_cast<int>(anim.getVirtualY() * TILE_SIZE);
        const HealthBarLayout bar = computeHealthBar(entity.current_hp, entity.max_hp,
                                                     px - camera.x, py - camera.y, TILE_SIZE);
        if (!bar.visible) {
            return;
        }
        renderer.SetDrawColor(20, 20, 20, 255);
        renderer.FillRect(SDL2pp::Rect(bar.background.x, bar.background.y, bar.background.w,
                                       bar.background.h));
        if (bar.fill.w > 0) {
            renderer.Copy(barSheet, barSrc,
                          SDL2pp::Rect(bar.fill.x, bar.fill.y, bar.fill.w, bar.fill.h));
        }
    };
    for (const EntityDTO& player: lastSnapshot.players) {
        drawHealthBar(player);
    }
    for (const EntityDTO& monster: lastSnapshot.monsters) {
        drawHealthBar(monster);
    }
}
