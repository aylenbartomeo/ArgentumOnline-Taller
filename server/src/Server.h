#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>

#include "../../common/include/queue.h"

#include "Acceptor.h"
#include "dto/CommandDTO.h"
#include "ConnectionMonitor.h"
#include "GameLoop.h"

// #include "GameLoop.h"

/**
 * @class Server
 * @brief Orquestador principal del juego. Administra la cola central,
 * el hilo aceptador de conexiones y el GameLoop.
 */
class Server {
private:
    static constexpr const char* EXIT_CMD = "q";

    Queue<GameEvent> gameQueue;

    ConnectionMonitor monitor;

    GameLoop gameLoop;

    Acceptor acceptor;

    void wait_for_exit();

public:
    explicit Server(const char* port);

    void run();

    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
};

#endif
