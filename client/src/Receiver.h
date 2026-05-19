#ifndef CLIENT_RECEIVER_H
#define CLIENT_RECEIVER_H

#include "common/include/thread.h"
#include "common/src/protocol/Protocol.h"

class Client;

class Receiver: public Thread {
private:
    Protocol& protocol;
    Client& client;

public:
    Receiver(Protocol& protocol, Client& client);

    void run() override;

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
};

#endif
