#include "Receiver.h"

Receiver::Receiver(Socket& skt) : skt(skt) {}


void Receiver::run() {
    try {
        while (true) {
            uint8_t raw_opcode;
            skt.recvall(&raw_opcode, sizeof(uint8_t));

            OPCODE opcode = static_cast<OPCODE>(raw_opcode);

            switch (opcode) {
                case OPCODE::LOGIN: {
                    LoginDTO login = Protocol::receive_login(skt);
                    break;
                }
                case OPCODE::START_MOVE: {
                    StartMoveDTO startMove = Protocol::receive_start_move(skt);
                    // Nota: deberiamos de tener una cola ThreadSafeQueue compartida con GameLoop
                    // En lugar de que el DTO muera, se empuja a esta cola para ser procesado.
                    break;
                }
                default:
                    throw std::runtime_error("Opcode desconocido");
            }
        }
    } catch (const std::exception& e) {
        // cerrar conexión, loggear, etc.
    }
}
