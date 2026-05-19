#include "Client.h"

#include <iostream>

#include "common/include/dto/LoginDTO.h"

Client::Client(const char* hostname, const char* servname, const char* username):
        clientId(0),
        username(username),
        skt(hostname, servname),
        protocol(skt) {}

void Client::start() {
    LoginDTO loginDTO(this->username, "1234");
    protocol.send_login(loginDTO);
    std::cout << "[CLIENTE] conectado como " << this->username << std::endl;
}

Client::~Client() {}
