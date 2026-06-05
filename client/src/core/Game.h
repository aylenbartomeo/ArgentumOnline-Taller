#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>

#include "../animation/CharacterAnimator.h"
#include "../input/ChatCommandParser.h"
#include "../input/EventHandler.h"
#include "../rendering/TextureManager.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "../ui/MiniChat.h"
#include "../ui/Window.h"
#include "common/include/dto/Snapshot.h"

#include "Client.h"

class Game {
private:
    SDL2pp::SDL sdl;
    Window window;
    EventHandler events;
    Client& client;
    TextureManager textures;
    TileMap map;
    MiniChat miniChat;
    ChatCommandParser chatParser;
    SnapshotDTO lastSnapshot;
    Uint32 lastMoveSentMs;
    std::unordered_map<uint32_t, CharacterAnimator> animators;

    struct ActiveFx {
        uint32_t targetId;
        uint32_t startMs;
    };
    std::optional<ActiveFx> activeFx;

public:
    explicit Game(Client& client);
    ~Game() = default;

    void run();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    Game(Game&&) = default;
    Game& operator=(Game&&) = default;

private:
    void render(const FrameInput& input);
    void renderTerrain(const CameraOffset& camera);
    void renderOverlays(const CameraOffset& camera);
    void renderGroundItems(const CameraOffset& camera);
    void renderCitizens(const CameraOffset& camera);
    bool cellInSafeZone(int col, int row) const;
    void renderEntities(const CameraOffset& camera);
    CameraOffset computeCamera();
    void sendMoveIfDue(const FrameInput& input);
    void processCombatInput(const FrameInput& input, const CameraOffset& camera);
    void renderFx(const CameraOffset& camera);

    // Procesa el input del chat: si se confirmó un mensaje, lo envía al servidor y limpia el
    // buffer.
    void processChatInput(const FrameInput& input);

    // Procesa los inputs relacionados a comandos de trampa (cheats)
    void processCheats(const FrameInput& input);

    // Drena los mensajes de chat entrantes del servidor y los muestra en el MiniChat.
    void drainIncomingChat();

    // Intenta parsear un mensaje como chat privado. El formato esperado es: "/pm destinatario
    // mensaje".
    static CommandVariant buildChatCommand(const std::string& text);
};

#endif
