#include "Protocol.h"
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

Protocol::Protocol(Socket& skt) : skt(skt) {}

// =======================================================
// CAPA DE BAJO NIVEL
// =======================================================

void Protocol::send_uint8(uint8_t value) {
    skt.sendall(&value, sizeof(uint8_t));
}

void Protocol::send_uint16(uint16_t value) {
    uint16_t net_value = htons(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint16_t));
}
    
void Protocol::send_uint32(uint32_t value) {
    uint32_t net_value = htonl(value);

    skt.sendall(reinterpret_cast<const uint8_t*>(&net_value), sizeof(uint32_t));
}

void Protocol::send_string(const std::string& str) {
    uint16_t length = htons(static_cast<uint16_t>(str.size()));

    skt.sendall(reinterpret_cast<const uint8_t*>(&length), sizeof(uint16_t));
    skt.sendall(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

uint8_t Protocol::recv_uint8() {
    uint8_t value;
    skt.recvall(&value, sizeof(value)); 

    return value;
}

uint16_t Protocol::recv_uint16() {
    uint16_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohs(net_value);
}

uint32_t Protocol::recv_uint32() {
    uint32_t net_value;
    skt.recvall(&net_value, sizeof(net_value));

    return ntohl(net_value);
}

std::string Protocol::recv_string() {
    uint16_t len = recv_uint16();
    std::vector<char> buffer(len);

    skt.recvall(buffer.data(), len);

    return std::string(buffer.begin(), buffer.end());
}

// =======================================================
// CAPA SEMÁNTICA
// =======================================================

void Protocol::send_login(const LoginDTO& loginDTO) {
    send_uint8(static_cast<uint8_t>(OPCODE::LOGIN));

    send_string(loginDTO.username);
    send_string(loginDTO.password);
}

LoginDTO Protocol::receive_login() {
    std::string username = recv_string();
    std::string password = recv_string();

    return LoginDTO(username, password);
}

void Protocol::send_start_move(const StartMoveDTO& startMoveDTO) {
    send_uint8(static_cast<uint8_t>(OPCODE::START_MOVE));
    send_uint8(startMoveDTO.direction);
}

StartMoveDTO Protocol::receive_start_move() {
    uint8_t dir = recv_uint8();

    return StartMoveDTO(static_cast<Direction>(dir));
}