#ifndef SERVER_H
#define SERVER_H

#include <mutex>
#include <string>
#include <vector>
#include "Acceptor.h"

/**
 * @class Server
 * @brief Orquestador principal que gestiona el ciclo de vida de los componentes del servidor.
 * * Administra la sincronización entre el Acceptor y el GameLoop, coordinando el inicio,
 * la ejecución y el apagado controlado del sistema.
 */
class Server {
private:
    static constexpr size_t QUEUE_CAPACITY = 10000;
    static constexpr const char* EXIT_CMD = "q";

    Acceptor acceptor;

    /**
     * @brief Inicializa y lanza el hilo del Acceptor para comenzar a recibir conexiones.
     */
    void start_acceptor();

    /**
     * @brief Inicializa y lanza el hilo del GameLoop para comenzar el procesamiento de la lógica.
     */
    void start_game_loop();

    /**
     * @brief Bloquea el hilo principal y espera la instrucción de salida por entrada estándar.
     */
    void wait_for_exit();

public:
    explicit Server(const char* port);

    /* Deshabilito las copias */
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    /* Permito movimiento */
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;

    void run();

    ~Server();
};

#endif
