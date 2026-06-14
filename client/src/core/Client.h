#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>
#include <optional>
#include <string>

#include "../network/Receiver.h"
#include "../network/Sender.h"
#include "common/include/dto/ClientCommands.h"
#include "common/include/dto/JoinResponseDTO.h"
#include "common/include/dto/PlayerStatsDTO.h"
#include "common/include/dto/Snapshot.h"
#include "common/include/queue.h"
#include "common/src/protocol/Protocol.h"

class Client {
private:
    uint32_t clientId;
    std::string username;
    Socket skt;
    Protocol protocol;

    Queue<SnapshotDTO> snapshotQueue;
    Queue<ChatDTO> chatQueue;
    Queue<PlayerStatsDTO> statsQueue;
    Queue<JoinResponseDTO> joinResponseQueue;
    Queue<CommandVariant> commandQueue;

    Receiver receiver;
    Sender sender;
    bool wasStarted;

    std::optional<uint32_t> selectedNpcId;
    std::string selectedNpcType = "dynamic";

public:
    explicit Client(const char* hostname, const char* servname);

    void start();
    void stop();

    void pushSnapshot(const SnapshotDTO& snap);
    bool tryPopSnapshot(SnapshotDTO& out);

    void pushChatMessage(const ChatDTO& chat);
    bool tryPopChatMessage(ChatDTO& out);

    void pushPlayerStats(const PlayerStatsDTO& stats);
    bool tryPopPlayerStats(PlayerStatsDTO& out);

    void pushJoinResponse(const JoinResponseDTO& dto);
    bool tryPopJoinResponse(JoinResponseDTO& out);

    void sendCommand(const CommandVariant& cmd);

    void setSelectedNpc(std::optional<uint32_t> id, const std::string& type = "dynamic") {
        selectedNpcId = id;
        selectedNpcType = type;
    }

    std::optional<uint32_t> getSelectedNpc() const { return selectedNpcId; }
    std::string getSelectedNpcType() const { return selectedNpcType; }

    uint32_t getClientId() const;

    bool authenticate(const std::string& action, const std::string& username,
                      const std::string& password, std::string& errorMessage);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    ~Client();
};

#endif
