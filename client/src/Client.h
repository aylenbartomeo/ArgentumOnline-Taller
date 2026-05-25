#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>
#include <string>

#include "common/include/dto/ClientCommands.h"
#include "common/include/dto/Snapshot.h"
#include "common/include/queue.h"
#include "common/src/protocol/Protocol.h"

#include "Receiver.h"
#include "Sender.h"

class Client {
private:
    uint32_t clientId;
    std::string username;
    Socket skt;
    Protocol protocol;

    Queue<SnapshotDTO> snapshotQueue;
    Queue<ChatDTO> chatQueue;
    Queue<CommandVariant> commandQueue;

    Receiver receiver;
    Sender sender;
    bool wasStarted;

public:
    explicit Client(const char* hostname, const char* servname);

    void start();
    void stop();

    void pushSnapshot(const SnapshotDTO& snap);
    bool tryPopSnapshot(SnapshotDTO& out);

    void pushChatMessage(const ChatDTO& chat);
    bool tryPopChatMessage(ChatDTO& out);

    void sendCommand(const CommandVariant& cmd);

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
