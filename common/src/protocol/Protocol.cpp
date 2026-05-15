#include "Protocol.h"
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <stdexcept>

void Protocol::send_login(const LoginDTO& loginDTO, Socket& skt) {
    std::vector<uint8_t> buffer;

    buffer.push_back(static_cast<uint8_t>(OPCODE::LOGIN));

    uint16_t user_len = htons(static_cast<uint16_t>(loginDTO.username.size()));

    const uint8_t* user_len_ptr = reinterpret_cast<const uint8_t*>(&user_len);
    buffer.insert(buffer.end(), user_len_ptr, user_len_ptr + sizeof(uint16_t));

    buffer.insert(buffer.end(), loginDTO.username.begin(), loginDTO.username.end());

    uint16_t pass_len = htons(static_cast<uint16_t>(loginDTO.password.size()));
    const uint8_t* pass_len_ptr = reinterpret_cast<const uint8_t*>(&pass_len);
    buffer.insert(buffer.end(), pass_len_ptr, pass_len_ptr + sizeof(uint16_t));

    buffer.insert(buffer.end(), loginDTO.password.begin(), loginDTO.password.end());

    skt.sendall(buffer.data(), buffer.size());
}

LoginDTO Protocol::receive_login(Socket& skt) {
    uint16_t user_len;
    skt.recvall(reinterpret_cast<uint8_t*>(&user_len), sizeof(uint16_t));
    user_len = ntohs(user_len);

    std::vector<char> user_buffer(user_len);
    skt.recvall(reinterpret_cast<uint8_t*>(user_buffer.data()), user_len);
    std::string username(user_buffer.begin(), user_buffer.end());

    uint16_t pass_len;
    skt.recvall(reinterpret_cast<uint8_t*>(&pass_len), sizeof(uint16_t));
    pass_len = ntohs(pass_len);

    std::vector<char> pass_buffer(pass_len);
    skt.recvall(reinterpret_cast<uint8_t*>(pass_buffer.data()), pass_len);
    std::string password(pass_buffer.begin(), pass_buffer.end());

    return LoginDTO(username, password);
}

void Protocol::send_start_move(const StartMoveDTO& startMoveDTO, Socket& skt) {
    uint8_t opcode = static_cast<uint8_t>(OPCODE::START_MOVE);
    uint8_t direction = startMoveDTO.direction;

    skt.sendall(&opcode, sizeof(uint8_t));
    skt.sendall(&direction, sizeof(uint8_t));
}

StartMoveDTO Protocol::receive_start_move(Socket& skt) {
    uint8_t direction;
    skt.recvall(&direction, sizeof(uint8_t));
    return StartMoveDTO(static_cast<Direction>(direction));
}