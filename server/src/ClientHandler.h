#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "../../common/src/socket/socket.h"

#include "Receiver.h"
#include "Sender.h"

class ClientHandler {
private:
    uint32_t client_id;
    Socket skt;

    Queue<SnapshotDTO> sender_queue;

    Receiver receiver;
    Sender sender;

public:
    explicit ClientHandler(Socket&& skt, uint32_t client_id, Queue<GameEvent>& gameQueue);

    void startThreads();
    void stopThreads();
    void joinThreads();

    bool isOnline() const;

    // El GameLoop usará este método en el momento del LOGIN para registrar adónde enviarle los
    // datos
    Queue<SnapshotDTO>& getSenderQueue();
    uint32_t getId() const;

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
};

#endif
