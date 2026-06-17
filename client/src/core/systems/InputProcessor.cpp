#include "InputProcessor.h"

#include <algorithm>
#include <optional>

#include <SDL2/SDL.h>

#include "../animation/Death.h"
#include "../common/GameConstants.h"
#include "../common/WeaponHelper.h"
#include "../input/NpcPolicy.h"
#include "../rendering/NpcVisuals.h"
#include "common/include/dto/CheatDTO.h"
#include "common/include/dto/ClientCommands.h"
#include "common/include/dto/StartMoveDTO.h"

namespace GC = GameConstants;

namespace {
constexpr Uint32 MOVE_INTERVAL_MS = 200;
constexpr int ATTACK_RANGE_TILES = 1;
constexpr int MAX_SELECT_RANGE = 2;
}  // namespace

// ─── Constructor ──────────────────────────────────────────────────────────────

InputProcessor::InputProcessor(Client& client, Window& window, MiniChat& miniChat, HudPanel& hud,
                               ManualPanel& manualPanel, ChatCommandParser& chatParser):
        client(client),
        window(window),
        miniChat(miniChat),
        hud(hud),
        manualPanel(manualPanel),
        chatParser(chatParser) {}

// ─── Chat ─────────────────────────────────────────────────────────────────────

// Retorna true si el comando es un ChatDTO sentinel de error (emitido por el parser).
static bool handleParseErrors(const CommandVariant& cmd, MiniChat& miniChat) {
    if (!std::holds_alternative<ChatDTO>(cmd))
        return false;
    const std::string& msg = std::get<ChatDTO>(cmd).message;
    if (msg == "__INVALID_NO_SLOT__") {
        miniChat.pushMessage("[INFO] Selecciona un slot del inventario primero.");
        return true;
    }
    if (msg == "__INVALID_ZERO_AMOUNT__") {
        miniChat.pushMessage("[INFO] La cantidad debe ser mayor a 0.");
        return true;
    }
    if (msg == "__INVALID_AMOUNT_PARSE__") {
        miniChat.pushMessage("[INFO] Cantidad invalida.");
        return true;
    }
    return false;
}

bool InputProcessor::validateAndAnnotateNpcCommand(NpcCommandDTO& npcCmd) {
    auto target = client.getSelectedNpc();
    if (!target) {
        miniChat.pushMessage("[INFO] Debes de seleccionar un NPC primero.");
        return false;
    }
    npcCmd.npcId = *target;

    const std::string npcType = client.getSelectedNpcType();
    auto ruleOpt = NpcPolicy::validate(npcType, npcCmd.type);
    if (!ruleOpt) {
        miniChat.pushMessage("[INFO] El " + NpcVisuals::displayName(npcType) +
                             " no ofrece ese servicio.");
        return false;
    }
    miniChat.pushMessage(ruleOpt->feedbackMsg);
    return true;
}

void InputProcessor::processChatInput(const FrameInput& input, AudioSystem& audio) {
    if (!input.chatSubmitted || input.chatText.empty())
        return;

    auto cmdOpt = chatParser.parse(input.chatText);
    if (!cmdOpt) {
        miniChat.pushMessage("[Info] Comando inexistente o mal formateado.");
        return;
    }

    CommandVariant& cmd = *cmdOpt;

    if (handleParseErrors(cmd, miniChat))
        return;

    if (std::holds_alternative<UseItemDTO>(cmd))
        audio.playSound(SoundEffect::DRINK_POTION);

    if (std::holds_alternative<NpcCommandDTO>(cmd)) {
        NpcCommandDTO& npcCmd = std::get<NpcCommandDTO>(cmd);
        if (!validateAndAnnotateNpcCommand(npcCmd))
            return;
    }

    client.sendCommand(cmd);
}

// ─── Cheats ───────────────────────────────────────────────────────────────────

void InputProcessor::processCheats(const FrameInput& input) {
    if (input.cheatLevelUp)
        client.sendCommand(CheatDTO{CheatType::LEVEL_UP});
    if (input.cheatDie)
        client.sendCommand(CheatDTO{CheatType::DIE});
    if (input.cheatGiveRanged)
        client.sendCommand(CheatDTO{CheatType::GIVE_WEAPONS});
    if (input.cheatInfiniteMana) {
        client.sendCommand(CheatDTO{CheatType::INFINITE_MANA});
        localInfiniteManaActive = !localInfiniteManaActive;
    }
    if (input.cheatGiveGold)
        client.sendCommand(CheatDTO{CheatType::GIVE_GOLD});
    if (input.cheatGiveArmors)
        client.sendCommand(CheatDTO{CheatType::GIVE_ARMORS});
}

// ─── Equip / Use / Slot / UI ──────────────────────────────────────────────────

void InputProcessor::processEquipInput(const FrameInput& input) {
    if (!input.equipPressed)
        return;
    const int slot = hud.slotAtPosition(input.equipX, input.equipY);
    if (slot >= 0) {
        client.sendCommand(EquipItemDTO{static_cast<uint8_t>(slot)});
        if (hud.getSelectedSlot() != slot)
            hud.selectSlot(slot);
    }
}

void InputProcessor::processUseInput(const FrameInput& input, AudioSystem& audio) {
    if (!input.consumeKeyPressed)
        return;
    const int slot = hud.getSelectedSlot();
    if (slot >= 0) {
        client.sendCommand(UseItemDTO{static_cast<uint8_t>(slot)});
        audio.playSound(SoundEffect::DRINK_POTION);
    } else {
        miniChat.pushMessage("[Info] Selecciona un slot del inventario primero.");
    }
}

void InputProcessor::processGrabDropInput(const FrameInput& input) {
    if (input.grabKeyPressed) {
        client.sendCommand(GrabItemDTO{});
    }

    if (input.dropKeyPressed) {
        const int slot = hud.getSelectedSlot();
        if (slot >= 0) {
            // Amount 0 implies dropping the whole stack by default in /tirar behavior if args empty
            client.sendCommand(DropItemDTO{static_cast<uint8_t>(slot), 0});
        } else {
            miniChat.pushMessage("[Info] Selecciona un slot del inventario primero.");
        }
    }
}

void InputProcessor::processSelectSlotInput(const FrameInput& input) {
    if (!input.mouseLeftJustPressed || input.equipPressed)
        return;
    const int slot = hud.slotAtPosition(input.mouseX, input.mouseY);
    if (slot >= 0)
        hud.selectSlot(slot);
}

void InputProcessor::processUiInput(const FrameInput& input) {
    if (!input.mouseLeftJustPressed)
        return;
    if (hud.isManualButtonClicked(input.mouseX, input.mouseY))
        manualPanel.toggle();
}

// ─── Chat drain / Move ────────────────────────────────────────────────────────

void InputProcessor::drainIncomingChat() {
    ChatDTO chat;
    while (client.tryPopChatMessage(chat)) miniChat.pushMessage(chat.message);
}

void InputProcessor::sendMoveIfDue(const FrameInput& input, const SnapshotDTO& snapshot,
                                   const TileMap& map) {
    if (input.chatInputActive)
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
        client.setSelectedNpc(std::nullopt);
    }
}

// ─── Combat ───────────────────────────────────────────────────────────────────

InputProcessor::CombatResult InputProcessor::processCombatInput(const FrameInput& input,
                                                                const CameraOffset& camera,
                                                                const SnapshotDTO& snapshot,
                                                                const PlayerStatsDTO& stats,
                                                                const TileMap& map) {
    CombatResult result;

    if (input.resurrectPressed)
        client.sendCommand(ResurrectDTO{});

    const EntityDTO* localPlayer = findEntityById(snapshot, client.getClientId());
    if (!localPlayer || isDead(localPlayer->current_hp))
        return result;

    const bool inSafeZone = std::any_of(
            map.getSafeZones().begin(), map.getSafeZones().end(), [&](const SafeZoneRect& zone) {
                return localPlayer->x >= zone.x && localPlayer->x < zone.x + zone.width &&
                       localPlayer->y >= zone.y && localPlayer->y < zone.y + zone.height;
            });

    if (input.shootPressed) {
        if (inSafeZone) {
            miniChat.pushMessage("[INFO] Estás en zona segura.");
            return result;
        }
        if (WeaponHelper::hasFlauta(stats)) {
            if (stats.currentMana > 0 || localInfiniteManaActive) {
                result.fx =
                        ActiveFx{client.getClientId(), SDL_GetTicks(), 0, 0, FxType::FLAUTA_HEAL};
                client.sendCommand(ShootDTO{0.f, 0.f});
            } else {
                miniChat.pushMessage("[INFO] No tienes maná suficiente.");
            }
        } else {
            const bool isBow = WeaponHelper::hasBow(stats);
            const bool isMagic = !isBow;
            if (isMagic && stats.currentMana <= 0 && !localInfiniteManaActive) {
                miniChat.pushMessage("[INFO] No tienes maná suficiente.");
            } else {
                const Cell cell = screenToCell(input.shootScreenX, input.shootScreenY, camera.x,
                                               camera.y, GC::TILE_SIZE);
                client.sendCommand(
                        ShootDTO{static_cast<float>(cell.col), static_cast<float>(cell.row)});
                if (isBow)
                    result.bowAttack = true;
                else
                    result.magicAttack = true;
            }
        }
    }

    if (!input.attackPressed)
        return result;

    if (miniChat.isMouseOver(input.attackX, input.attackY, GC::WINDOW_HEIGHT))
        return result;

    const Cell cell = screenToCell(input.attackX, input.attackY, camera.x, camera.y, GC::TILE_SIZE);

    // Si el click cayó sobre un ciudadano, ignorar el ataque melee
    const bool clickedCitizen = std::any_of(
            map.getCitizens().begin(), map.getCitizens().end(), [&](const MapCitizen& c) {
                return c.x == cell.col && (c.y == cell.row || c.y - 1 == cell.row);
            });
    if (clickedCitizen)
        return result;

    if (inSafeZone) {
        miniChat.pushMessage("[INFO] Estás en zona segura.");
        return result;
    }

    const bool targetInSafeZone = std::any_of(
            map.getSafeZones().begin(), map.getSafeZones().end(), [&](const SafeZoneRect& zone) {
                return cell.col >= zone.x && cell.col < zone.x + zone.width && cell.row >= zone.y &&
                       cell.row < zone.y + zone.height;
            });
    if (targetInSafeZone) {
        miniChat.pushMessage("[INFO] El objetivo está en zona segura.");
        return result;
    }

    const auto target =
            pickTargetAt(cell.col, cell.row, snapshot, client.getClientId(), ATTACK_RANGE_TILES);
    if (target) {
        client.sendCommand(AttackDTO{*target});
        FxType fxType = WeaponHelper::hasFlauta(stats) ? FxType::FLAUTA_HEAL :
                        WeaponHelper::hasSword(stats)  ? FxType::SWORD :
                                                         FxType::DEFAULT;
        result.fx = ActiveFx{*target, SDL_GetTicks(), 0, 0, fxType};
    }

    return result;
}

// ─── NPC target input ─────────────────────────────────────────────────────────

void InputProcessor::processNpcTargetInput(const FrameInput& input, const CameraOffset& camera,
                                           const SnapshotDTO& snapshot, const TileMap& map) {
    if (!input.mouseLeftJustPressed || input.equipPressed)
        return;
    if (miniChat.isMouseOver(input.mouseX, input.mouseY, GC::WINDOW_HEIGHT))
        return;

    const Cell cell = screenToCell(input.mouseX, input.mouseY, camera.x, camera.y, GC::TILE_SIZE);

    const auto citizenIt = std::find_if(
            map.getCitizens().begin(), map.getCitizens().end(), [&cell](const auto& cit) {
                return cit.x == cell.col && (cit.y == cell.row || cit.y - 1 == cell.row);
            });

    if (citizenIt == map.getCitizens().end())
        return;

    // Verificar rango máximo de selección
    const EntityDTO* me = findEntityById(snapshot, client.getClientId());
    if (!me)
        return;
    const int dist = std::max(std::abs(me->x - citizenIt->x), std::abs(me->y - citizenIt->y));
    if (dist > MAX_SELECT_RANGE) {
        miniChat.pushMessage("[INFO] Estás demasiado lejos para interactuar.");
        return;
    }

    // Deseleccionar si se vuelve a clickear el mismo NPC
    const uint32_t fakeId = NpcVisuals::encodeId(citizenIt->x, citizenIt->y);
    const auto monsterIt = std::find_if(
            snapshot.monsters.begin(), snapshot.monsters.end(), [&cell](const auto& m) {
                return m.x == cell.col && (m.y == cell.row || m.y - 1 == cell.row);
            });
    const uint32_t targetId = (monsterIt != snapshot.monsters.end()) ? monsterIt->id : fakeId;

    if (client.getSelectedNpc() == targetId) {
        client.setSelectedNpc(std::nullopt);
        miniChat.pushMessage("[INFO] NPC deseleccionado.");
        return;
    }

    client.setSelectedNpc(targetId, citizenIt->type);
    client.sendCommand(AttackDTO{targetId});
    miniChat.pushMessage("[INFO] Seleccionaste al " + NpcVisuals::displayName(citizenIt->type) +
                         ".");
}
