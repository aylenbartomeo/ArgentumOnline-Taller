#include "InputProcessor.h"

#include <optional>

#include <SDL2/SDL.h>

#include "../animation/Death.h"
#include "../common/GameConstants.h"
#include "../common/WeaponHelper.h"
#include "common/include/dto/CheatDTO.h"
#include "common/include/dto/ClientCommands.h"
#include "common/include/dto/StartMoveDTO.h"

namespace GC = GameConstants;

namespace {
constexpr Uint32 MOVE_INTERVAL_MS = 200;
constexpr int ATTACK_RANGE_TILES = 1;
}  // namespace

InputProcessor::InputProcessor(Client& client, Window& window, MiniChat& miniChat, HudPanel& hud,
                               ManualPanel& manualPanel, ChatCommandParser& chatParser):
        client(client),
        window(window),
        miniChat(miniChat),
        hud(hud),
        manualPanel(manualPanel),
        chatParser(chatParser) {}

void InputProcessor::processChatInput(const FrameInput& input) {
    if (!input.chatSubmitted || input.chatText.empty())
        return;
    std::optional<CommandVariant> cmdOpt = chatParser.parse(input.chatText);
    if (!cmdOpt.has_value()) {
        miniChat.pushMessage("[Info] Comando inexistente o mal formateado.");
        return;
    }

    // Interceptar errores de /tirar (señalizados como ChatDTO con prefijo especial)
    if (std::holds_alternative<ChatDTO>(cmdOpt.value())) {
        const std::string& msg = std::get<ChatDTO>(cmdOpt.value()).message;
        if (msg == "__INVALID_NO_SLOT__") {
            miniChat.pushMessage("[Info] Selecciona un slot del inventario primero.");
            return;
        }
        if (msg == "__INVALID_ZERO_AMOUNT__") {
            miniChat.pushMessage("[Info] La cantidad debe ser mayor a 0.");
            return;
        }
        if (msg == "__INVALID_AMOUNT_PARSE__") {
            miniChat.pushMessage("[Info] Cantidad invalida.");
            return;
        }
    }

    client.sendCommand(cmdOpt.value());
}

void InputProcessor::processCheats(const FrameInput& input) {
    if (input.cheatLevelUp)
        client.sendCommand(CheatDTO{CheatType::LEVEL_UP});
    if (input.cheatDie)
        client.sendCommand(CheatDTO{CheatType::DIE});
    if (input.cheatGiveRanged)
        client.sendCommand(CheatDTO{CheatType::GIVE_RANGED_WEAPONS});
    if (input.cheatInfiniteMana)
        client.sendCommand(CheatDTO{CheatType::INFINITE_MANA});
    if (input.cheatGiveGold)
        client.sendCommand(CheatDTO{CheatType::GIVE_GOLD});
}

void InputProcessor::processEquipInput(const FrameInput& input) {
    if (!input.equipPressed)
        return;
    float lx = 0.f, ly = 0.f;
    SDL_RenderWindowToLogical(window.getRenderer().Get(), input.equipX, input.equipY, &lx, &ly);
    const int slot = hud.slotAtPosition(input.equipX, input.equipY);
    if (slot >= 0)
        client.sendCommand(EquipItemDTO{static_cast<uint8_t>(slot)});
}

void InputProcessor::processUseInput(const FrameInput& input) {
    if (!input.consumeKeyPressed)
        return;

    int slot = hud.getSelectedSlot();
    if (slot >= 0) {
        client.sendCommand(UseItemDTO{static_cast<uint8_t>(slot)});
    } else {
        miniChat.pushMessage("[Info] Selecciona un slot del inventario primero.");
    }
}

void InputProcessor::processSelectSlotInput(const FrameInput& input) {
    // Solo procesar click simple (no doble click que va a equip)
    if (!input.mouseLeftJustPressed || input.equipPressed)
        return;

    const int slot = hud.slotAtPosition(input.mouseX, input.mouseY);
    if (slot >= 0) {
        // selectSlot internamente hace toggle si es el mismo slot
        hud.selectSlot(slot);
    }
}

void InputProcessor::processUiInput(const FrameInput& input) {
    if (!input.mouseLeftJustPressed)
        return;
    float lx = 0.f, ly = 0.f;
    SDL_RenderWindowToLogical(window.getRenderer().Get(), input.mouseX, input.mouseY, &lx, &ly);
    if (hud.isManualButtonClicked(static_cast<int>(lx), static_cast<int>(ly)))
        manualPanel.toggle();
}

void InputProcessor::drainIncomingChat() {
    ChatDTO chat;
    while (client.tryPopChatMessage(chat)) miniChat.pushMessage(chat.message);
}

void InputProcessor::sendMoveIfDue(const FrameInput& input, const SnapshotDTO& snapshot) {
    if (input.chatInputActive)
        return;
    const EntityDTO* localPlayer = findEntityById(snapshot, client.getClientId());
    if (localPlayer && isDead(localPlayer->current_hp))
        return;

    const Uint32 now = SDL_GetTicks();
    if (now - lastMoveSentMs < MOVE_INTERVAL_MS)
        return;

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

InputProcessor::CombatResult InputProcessor::processCombatInput(const FrameInput& input,
                                                                const CameraOffset& camera,
                                                                const SnapshotDTO& snapshot,
                                                                const PlayerStatsDTO& stats) {

    CombatResult result;

    if (input.resurrectPressed)
        client.sendCommand(ResurrectDTO{});

    const EntityDTO* localPlayer = findEntityById(snapshot, client.getClientId());
    if (localPlayer && isDead(localPlayer->current_hp))
        return result;

    if (input.shootPressed && WeaponHelper::hasFlauta(stats)) {
        result.fx = ActiveFx{client.getClientId(), SDL_GetTicks(), 0, 0, FxType::FLAUTA_HEAL};
        client.sendCommand(ShootDTO{0.f, 0.f});
    } else if (input.shootPressed) {
        float lx = 0.f, ly = 0.f;
        SDL_RenderWindowToLogical(window.getRenderer().Get(), input.shootScreenX,
                                  input.shootScreenY, &lx, &ly);
        const Cell cell = screenToCell(static_cast<int>(lx), static_cast<int>(ly), camera.x,
                                       camera.y, GC::TILE_SIZE);
        client.sendCommand(ShootDTO{static_cast<float>(cell.col), static_cast<float>(cell.row)});
    }

    if (!input.attackPressed)
        return result;

    float lx = 0.f, ly = 0.f;
    SDL_RenderWindowToLogical(window.getRenderer().Get(), input.attackX, input.attackY, &lx, &ly);
    const int mouseX = static_cast<int>(lx);
    const int mouseY = static_cast<int>(ly);

    if (miniChat.isMouseOver(mouseX, mouseY, GC::WINDOW_HEIGHT))
        return result;

    const Cell cell = screenToCell(mouseX, mouseY, camera.x, camera.y, GC::TILE_SIZE);
    const std::optional<uint32_t> target =
            pickTargetAt(cell.col, cell.row, snapshot, client.getClientId(), ATTACK_RANGE_TILES);

    if (target) {
        client.sendCommand(AttackDTO{*target});
        FxType fxType = FxType::DEFAULT;
        if (WeaponHelper::hasFlauta(stats))
            fxType = FxType::FLAUTA_HEAL;
        else if (WeaponHelper::hasSword(stats))
            fxType = FxType::SWORD;
        result.fx = ActiveFx{*target, SDL_GetTicks(), 0, 0, fxType};
    }

    return result;
}
