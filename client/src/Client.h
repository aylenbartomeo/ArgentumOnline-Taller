#ifndef CLIENT_H
#define CLIENT_H

#include <mutex>

#include "common/include/dto/Snapshot.h"
#include "common/src/protocol/Protocol.h"

#include "Receiver.h"

class Client {
private:
    int clientId;
    const char* username;
    Socket skt;
    Protocol protocol;

    std::mutex snapshotMutex;
    SnapshotDTO latestSnapshot;

    Receiver receiver;
    bool wasStarted;

public:
    explicit Client(const char* hostname, const char* servname, const char* username);

    void start();
    void stop();

    void updateSnapshot(const SnapshotDTO& snap);
    SnapshotDTO getLatestSnapshot();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    ~Client();
};

#endif
