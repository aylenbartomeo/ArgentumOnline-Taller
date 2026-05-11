#include "Acceptor.h"

#define EXE_KNOWN_MSG "Error: "

Acceptor::Acceptor(const char* port) :
    skt(port) {}

void Acceptor::reap() {
    // Implementar la lógica para identificar y eliminar clientes desconectados.
}

void Acceptor::close_clients() {
    // Implementar la lógica para cerrar todas las conexiones activas y limpiar la lista de clientes.
}

void Acceptor::run() {
    // ACA inicializariamos el monitor
    // Falta implementar el ClientHandler, por ende, el metodo run() queda asi por ahora.
    while (should_keep_running()) {
        try {
            Socket client_skt = skt.accept();
            clients.push_back(new ClientHandler(std::move(client_skt)));
        } catch (const std::exception& e) {
            if (should_keep_running()) {
                std::cerr << EXE_KNOWN_MSG << e.what() << std::endl;
            }
            close_clients();
        }
    }
}

void Acceptor::stop() {
    Thread::stop();
    skt.shutdown(SHUT_RDWR);
    skt.close();
}
