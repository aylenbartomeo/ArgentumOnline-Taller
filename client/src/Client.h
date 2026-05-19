#ifndef CLIENT_H
#define CLIENT_H

#include "common/include/dto/Snapshot.h"
#include "common/include/queue.h"
#include "common/src/protocol/Protocol.h"

#include "Receiver.h"

class Client {
private:
    int clientId;
    const char* username;
    Socket skt;
    Protocol protocol;

    Queue<SnapshotDTO> snapshotQueue;

    Receiver receiver;
    bool wasStarted;

public:
    explicit Client(const char* hostname, const char* servname, const char* username);

    void start();
    void stop();

    void pushSnapshot(const SnapshotDTO& snap);
    bool tryPopSnapshot(SnapshotDTO& out);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    ~Client();
};

#endif
