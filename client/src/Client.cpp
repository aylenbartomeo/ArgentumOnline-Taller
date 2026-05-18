#include "Client.h"

Client::Client(const char* hostname, const char* servname, const char* username):
        clientId(0),
        username(username),
        skt(hostname, servname),
        protocol(skt) {}

void Client::start() {}

Client::~Client() {}
