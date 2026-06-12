#include "InputProcessor.h"

#include <algorithm>
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

void InputProcessor::processChatInput(const FrameInput& input, AudioSystem& audio) {
    if (!input.chatSubmitted || input.chatText.empty())
        return;
    std::optional<CommandVariant> cmdOpt = chatParser.parse(input.chatText);
    if (!cmdOpt.has_value()) {
        miniChat.pushMessage("[Info] Comando inexistente o mal formateado.");
        return;
    }

    CommandVariant cmd = cmdOpt.value();

    if (std::holds_alternative<ChatDTO>(cmd)) {
        const std::string& msg = std::get<ChatDTO>(cmd).message;
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

    if (std::holds_alternative<UseItemDTO>(cmd)) {
        audio.playSound(SoundEffect::DRINK_POTION);
    }

    // --- Validación de Reglas de Negocio y Feedback Local ---
    if (std::holds_alternative<NpcCommandDTO>(cmd)) {
        NpcCommandDTO& npcCmd = std::get<NpcCommandDTO>(cmd);

        if (npcCmd.type != LIST) {
            auto target = client.getSelectedNpc();
            if (!target) {
                miniChat.pushMessage("[INFO] Debes de seleccionar un NPC primero.");
                return;
            }

            // Inyectamos el target real antes de enviarlo por la red
            npcCmd.npcId = *target;
            std::string type = client.getSelectedNpcType();

            if (type == "priest") {
                if (npcCmd.type != HEAL && npcCmd.type != BUY) {
                    miniChat.pushMessage("[INFO] El Sacerdote no ofrece ese servicio.");
                    return;
                }
                if (npcCmd.type == HEAL)
                    miniChat.pushMessage("[INFO] Pidiendo curación a los dioses...");
                if (npcCmd.type == BUY)
                    miniChat.pushMessage("[INFO] Comprando artículos sagrados...");
            } else if (type == "merchant") {
                if (npcCmd.type != BUY && npcCmd.type != SELL) {
                    miniChat.pushMessage("[INFO] El Comerciante solo compra y vende.");
                    return;
                }
                if (npcCmd.type == BUY)
                    miniChat.pushMessage("[INFO] Comprando mercancía...");
                if (npcCmd.type == SELL)
                    miniChat.pushMessage("[INFO] Vendiendo mercancía...");
            } else if (type == "banker") {
                if (npcCmd.type != DEPOSIT && npcCmd.type != WITHDRAW) {
                    miniChat.pushMessage("[INFO] El Banquero solo acepta depósitos o retiros.");
                    return;
                }
                if (npcCmd.type == DEPOSIT)
                    miniChat.pushMessage("[INFO] Depositando en tu bóveda...");
                if (npcCmd.type == WITHDRAW)
                    miniChat.pushMessage("[INFO] Retirando de tu bóveda...");
            }
        }
    }

    client.sendCommand(cmd);
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
    localInfiniteManaActive = !localInfiniteManaActive;
    if (input.cheatGiveGold)
        client.sendCommand(CheatDTO{CheatType::GIVE_GOLD});
}

void InputProcessor::processEquipInput(const FrameInput& input) {
    if (!input.equipPressed)
        return;

    const int slot = hud.slotAtPosition(input.equipX, input.equipY);

    if (slot >= 0) {
        client.sendCommand(EquipItemDTO{static_cast<uint8_t>(slot)});
        if (hud.getSelectedSlot() != slot) {
            hud.selectSlot(slot);
        }
    }
}

void InputProcessor::processUseInput(const FrameInput& input, AudioSystem& audio) {
    if (!input.consumeKeyPressed)
        return;

    int slot = hud.getSelectedSlot();
    if (slot >= 0) {
        client.sendCommand(UseItemDTO{static_cast<uint8_t>(slot)});
        audio.playSound(SoundEffect::DRINK_POTION);
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
    if (hud.isManualButtonClicked(input.mouseX, input.mouseY))
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
                                                                const PlayerStatsDTO& stats,
                                                                const TileMap& map) {
    CombatResult result;

    if (input.resurrectPressed)
        client.sendCommand(ResurrectDTO{});

    const EntityDTO* localPlayer = findEntityById(snapshot, client.getClientId());
    if (!localPlayer || isDead(localPlayer->current_hp))
        return result;

    bool inSafeZone = std::any_of(
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
            bool isBow = WeaponHelper::hasBow(stats);
            bool isMagic = !isBow;

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

    // Lógica MELEE
    if (!input.attackPressed)
        return result;

    const int mouseX = input.attackX;
    const int mouseY = input.attackY;

    if (miniChat.isMouseOver(mouseX, mouseY, GC::WINDOW_HEIGHT))
        return result;

    const Cell cell = screenToCell(mouseX, mouseY, camera.x, camera.y, GC::TILE_SIZE);

    // INTERCEPCIÓN: Verificamos si le hizo click a un NPC Estático (Priest, Banker, Merchant)
    bool clickedCitizen = std::any_of(
            map.getCitizens().begin(), map.getCitizens().end(), [&](const MapCitizen& c) {
                return c.x == cell.col && (c.y == cell.row || c.y - 1 == cell.row);
            });

    if (clickedCitizen) {
        return result;  // Ignoramos el ataque local para que no diga "Estás en zona segura",
    }

    // Si no fue un NPC, entonces sí es un ataque normal. Verificamos zona segura:
    if (inSafeZone) {
        miniChat.pushMessage("[INFO] Estás en zona segura.");
        return result;
    }

    bool targetInSafeZone = std::any_of(
            map.getSafeZones().begin(), map.getSafeZones().end(), [&](const SafeZoneRect& zone) {
                return cell.col >= zone.x && cell.col < zone.x + zone.width && cell.row >= zone.y &&
                       cell.row < zone.y + zone.height;
            });

    if (targetInSafeZone) {
        miniChat.pushMessage("[INFO] El objetivo está en zona segura.");
        return result;
    }

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

void InputProcessor::processNpcTargetInput(const FrameInput& input, const CameraOffset& camera,
                                           const SnapshotDTO& snapshot, const TileMap& map) {
    if (!input.mouseLeftJustPressed || input.equipPressed)
        return;
    if (miniChat.isMouseOver(input.mouseX, input.mouseY, GC::WINDOW_HEIGHT))
        return;

    const Cell cell = screenToCell(input.mouseX, input.mouseY, camera.x, camera.y, GC::TILE_SIZE);

    // 1. Verificamos si es un NPC basándonos en el mapa
    const auto citizenIt = std::find_if(
            map.getCitizens().begin(), map.getCitizens().end(), [&cell](const auto& cit) {
                return cit.x == cell.col && (cit.y == cell.row || cit.y - 1 == cell.row);
            });

    if (citizenIt != map.getCitizens().end()) {
        const std::string& citType = citizenIt->type;

        const auto monsterIt = std::find_if(
                snapshot.monsters.begin(), snapshot.monsters.end(), [&cell](const auto& m) {
                    return m.x == cell.col && (m.y == cell.row || m.y - 1 == cell.row);
                });

        std::optional<uint32_t> realId;
        if (monsterIt != snapshot.monsters.end()) {
            realId = monsterIt->id;
        }

        uint32_t targetId = realId ? *realId : ((cell.col << 16) | cell.row);

        client.setSelectedNpc(targetId, citType);
        client.sendCommand(AttackDTO{targetId});
        miniChat.pushMessage("[Info] Seleccionaste al " + citType + ".");
    }
}
