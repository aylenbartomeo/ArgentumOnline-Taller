#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include "common/include/dto/ClientCommands.h"
#include "common/include/queue.h"
#include "common/include/thread.h"
#include "common/src/protocol/Protocol.h"

class Sender: public Thread {
private:
    Protocol& protocol;
    Queue<CommandVariant>& commandQueue;

public:
    Sender(Protocol& protocol, Queue<CommandVariant>& commandQueue);

    void run() override;

    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;
};

#endif
