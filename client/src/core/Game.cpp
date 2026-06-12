#include "Game.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "common/GameConstants.h"
#include "systems/StateAudioTrigger.h"

using GameConstants::VIEW_H;
using GameConstants::VIEW_W;
using GameConstants::VIEW_X;
using GameConstants::VIEW_Y;
using GameConstants::WINDOW_HEIGHT;
using GameConstants::WINDOW_WIDTH;

namespace {
constexpr const char* HUD_FONT_PATH = "resources/fonts/DejaVuSans.ttf";
constexpr const char* CHAT_FONT_PATH = "resources/fonts/DejaVuSans.ttf";

std::string readWholeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("No pude abrir el archivo: " + path);
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
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
        manualPanel(HUD_FONT_PATH),
        chatParser([this]() { return hud.getSelectedSlot(); },
                   [this]() { return this->client.getSelectedNpc(); }),
        lastSnapshot(),
        lastStats(),
        audio(),
        camera(),
        worldRenderer(textures, window.getRenderer(), map, worldFont),
        entityRenderer(textures, window.getRenderer(), client.getClientId()),
        fxSystem(textures, window.getRenderer()),
        inputProcessor(client, window, miniChat, hud, manualPanel, chatParser) {

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window.getRenderer().SetLogicalSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    worldFont = TTF_OpenFont(HUD_FONT_PATH, 12);
    if (!worldFont)
        std::cerr << "No pude abrir la fuente del texto del mundo: " << TTF_GetError() << std::endl;

    manualPanel.loadManual("../MANUAL_JUGADOR.md");
}

Game::~Game() {
    if (worldFont)
        TTF_CloseFont(worldFont);
}

void Game::run() {
    while (true) {
        const FrameInput input = events.pollEvents();
        if (input.quit)
            break;

        miniChat.update(input, VIEW_X + VIEW_W, VIEW_Y + VIEW_H);
        manualPanel.update(input, WINDOW_WIDTH, WINDOW_HEIGHT);

        inputProcessor.drainIncomingChat();
        inputProcessor.processChatInput(input, audio);
        inputProcessor.processCheats(input);
        inputProcessor.processEquipInput(input);
        inputProcessor.processUseInput(input, audio);
        inputProcessor.processSelectSlotInput(input);
        inputProcessor.processUiInput(input);
        if (input.toggleMute)
            audio.toggleMute();
        inputProcessor.sendMoveIfDue(input, lastSnapshot);

        render(input);
        audio.updateMonsterSounds(lastSnapshot, SDL_GetTicks(), client.getClientId());
        SDL_Delay(16);
    }
}

void Game::render(const FrameInput& input) {
    SnapshotDTO incomingSnap;
    PlayerStatsDTO incomingStats;
    PlayerStatsDTO previousStats = lastStats;

    while (client.tryPopSnapshot(incomingSnap)) lastSnapshot = incomingSnap;
    while (client.tryPopPlayerStats(incomingStats)) lastStats = incomingStats;

    StateAudioTrigger audioTrigger;
    StateChanges changes = audioTrigger.checkAndTrigger(previousStats, lastStats, audio);
    if (changes.tookDamage)
        fxSystem.triggerOnEntity(0, SDL_GetTicks(), FxType::BE_ATTACKED);
    if (changes.gotHealed)
        fxSystem.triggerOnEntity(0, SDL_GetTicks(), FxType::BE_HEALED);

    SDL2pp::Renderer& renderer = window.getRenderer();
    renderer.SetDrawColor(0, 0, 0, 255);
    renderer.Clear();

    const CameraOffset cam =
            camera.compute(client.getClientId(), lastSnapshot, entityRenderer.getAnimators(), map);

    // Procesar click izq sobre npc
    inputProcessor.processNpcTargetInput(input, cam, lastSnapshot, map);

    const auto combatResult =
            inputProcessor.processCombatInput(input, cam, lastSnapshot, lastStats, map);
    if (combatResult.fx) {
        fxSystem.triggerOnEntity(combatResult.fx->targetId, combatResult.fx->startMs,
                                 combatResult.fx->type);

        if (combatResult.fx->type == FxType::SWORD) {
            audio.playSound(SoundEffect::SWORD_ATTACK);
        } else if (combatResult.fx->type == FxType::FLAUTA_HEAL) {
            audio.playSound(SoundEffect::FLAUTE);
        }
    }

    if (combatResult.magicAttack)
        audio.playSound(SoundEffect::MAGIC_ATTACK);
    if (combatResult.bowAttack)
        audio.playSound(SoundEffect::BOW_SHOOT);

    const uint32_t now = SDL_GetTicks();

    if (fxSystem.syncProjectileAnimators(now, lastSnapshot))
        audio.playSound(SoundEffect::PROJ_HIT);

    worldRenderer.renderTerrain(cam);
    worldRenderer.renderOverlays(cam);
    worldRenderer.renderGroundItems(cam, lastSnapshot);
    worldRenderer.renderCitizens(cam, client.getSelectedNpc());
    entityRenderer.render(cam, lastSnapshot, now, client.getSelectedNpc());
    fxSystem.renderProjectiles(cam, now);
    fxSystem.render(cam, lastSnapshot, entityRenderer.getAnimators());
    fxSystem.renderFullscreen(WINDOW_WIDTH, WINDOW_HEIGHT);

    miniChat.render(renderer.Get(), VIEW_X + VIEW_W, VIEW_Y + VIEW_H, input.chatInputActive,
                    input.chatText);
    hud.renderBackground(renderer);
    hud.render(renderer, lastStats);
    manualPanel.render(renderer.Get(), WINDOW_WIDTH, WINDOW_HEIGHT);

    renderer.Present();
}
