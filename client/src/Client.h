#ifndef CLIENT_H
#define CLIENT_H

#include "common/src/protocol/Protocol.h"

class Client {
private:
    int clientId;
    const char* username;
    Socket skt;
    Protocol protocol;

public:
    explicit Client(const char* hostname, const char* servname, const char* username);

    void start();

    /* No permito copias */
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    /* Permito movimientos */
    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

    ~Client();
};

#endif
