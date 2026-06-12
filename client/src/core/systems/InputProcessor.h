#ifndef INPUT_PROCESSOR_H
#define INPUT_PROCESSOR_H

#include <optional>
#include <string>

#include "../../ui/HudPanel.h"
#include "../../ui/ManualPanel.h"
#include "../../ui/MiniChat.h"
#include "../../ui/Window.h"
#include "../Targeting.h"
#include "../common/FxType.h"
#include "../input/ChatCommandParser.h"
#include "../input/EventHandler.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/PlayerStatsDTO.h"
#include "common/include/dto/Snapshot.h"

#include "AudioSystem.h"
#include "Client.h"

class InputProcessor {
public:
    InputProcessor(Client& client, Window& window, MiniChat& miniChat, HudPanel& hud,
                   ManualPanel& manualPanel, ChatCommandParser& chatParser);

    void processChatInput(const FrameInput& input, AudioSystem& audio);
    void processCheats(const FrameInput& input);
    void processEquipInput(const FrameInput& input);
    void processUseInput(const FrameInput& input, AudioSystem& audio);
    void processSelectSlotInput(const FrameInput& input);
    void processUiInput(const FrameInput& input);
    void drainIncomingChat();
    void sendMoveIfDue(const FrameInput& input, const SnapshotDTO& snapshot);

    // Retorna el FX a activar si el usuario atacó/disparó, o std::nullopt si no.
    struct CombatResult {
        std::optional<ActiveFx> fx;
        bool magicAttack = false;
        bool bowAttack = false;
    };

    CombatResult processCombatInput(const FrameInput& input, const CameraOffset& camera,
                                    const SnapshotDTO& snapshot, const PlayerStatsDTO& stats,
                                    const TileMap& map);

    void processNpcTargetInput(const FrameInput& input, const CameraOffset& camera,
                               const SnapshotDTO& snapshot, const TileMap& map);

private:
    Client& client;
    Window& window;
    MiniChat& miniChat;
    HudPanel& hud;
    ManualPanel& manualPanel;
    ChatCommandParser& chatParser;
    uint32_t lastMoveSentMs = 0;
    bool localInfiniteManaActive = false;
};

#endif
