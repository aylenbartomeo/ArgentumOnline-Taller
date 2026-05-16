#include <gtest/gtest.h>

#include "../common/include/dto/LoginDTO.h"
#include "../common/src/CommandDTO.h"
#include "../common/src/protocol/Protocol.h"
#include "../common/src/socket/socket.h"

// --- TEST PARA EL MENSAJE DE LOGIN ---
TEST(ProtocolTest, LoginSerializationIsSymmetric) {
    LoginDTO original_dto("jugador1", "mi_password_secreta");

    Socket acceptor_skt("8080");
    Socket client_skt("localhost", "8080");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_login(original_dto);

    uint8_t action = server_protocol.receiveAction();
    std::string username = server_protocol.receiveUsername();
    std::string password = server_protocol.receivePassword();  // Leemos la pass también

    EXPECT_EQ(action, static_cast<uint8_t>(OPCODE::LOGIN));
    EXPECT_EQ(original_dto.username, username);
    EXPECT_EQ(original_dto.password, password);
}

// --- TEST PARA EL MENSAJE DE START_MOVE ---
TEST(ProtocolTest, StartMoveSerializationIsSymmetric) {
    // AHORA USAMOS TU COMMAND DTO DIRECTAMENTE
    CommandDTO original_dto;
    original_dto.type = ActionType::MOVE;
    original_dto.movement = Movement::UP;

    Socket acceptor_skt("8081");
    Socket client_skt("localhost", "8081");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // Enviamos usando el struct nuevo
    client_protocol.send_start_move(original_dto);

    uint8_t opcode;
    server_skt.recvall(&opcode, 1);

    CommandDTO received_dto;
    server_protocol.receive_start_move(received_dto);

    EXPECT_EQ(opcode, static_cast<uint8_t>(OPCODE::START_MOVE));
    EXPECT_EQ(static_cast<int>(original_dto.movement), static_cast<int>(received_dto.movement));
}
