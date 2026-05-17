#ifndef SENDER_H
#define SENDER_H

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "../../common/src/Snapshot.h"
#include "../../common/src/protocol/Protocol.h"
#include "../../common/src/socket/socket.h"

class Sender: public Thread {
private:
    Queue<SnapshotDTO>& senderQueue;
    Protocol protocol;

public:
    explicit Sender(Socket& skt, Queue<SnapshotDTO>& senderQueue);

    void run() override;

    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;

    Sender(Sender&&) = default;
    Sender& operator=(Sender&&) = default;
};

#endif
