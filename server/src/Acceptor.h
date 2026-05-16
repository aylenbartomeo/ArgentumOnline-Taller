#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <atomic>
#include <list>
#include <memory>
#include <utility>

#include "../../common/include/queue.h"
#include "../../common/include/thread.h"
#include "../../common/src/socket/socket.h"

#include "ClientHandler.h"
#include "CommandDTO.h"
#include "ConnectionMonitor.h"

class Acceptor: public Thread {
private:
    Socket skt;
    std::list<ClientHandler*> clients;
    Queue<GameEvent>& gameQueue;
    ConnectionMonitor& monitor;
    std::atomic<uint32_t> next_client_id{1};

    void reap();
    void close_clients();

public:
    Acceptor(const char* port, Queue<GameEvent>& gameQueue, ConnectionMonitor& monitor);

    virtual void run() override;
    virtual void stop() override;

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    Acceptor(Acceptor&&) = default;
    Acceptor& operator=(Acceptor&&) = default;
};

#endif
