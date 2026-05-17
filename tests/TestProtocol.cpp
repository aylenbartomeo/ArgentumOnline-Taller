#include <gtest/gtest.h>

#include "../common/include/dto/LoginDTO.h"
#include "../common/include/dto/StartMoveDTO.h"
#include "../common/include/dto/ClientCommands.h"
#include "../common/src/CommandDTO.h"
#include "../common/src/protocol/Protocol.h"
#include "../common/src/socket/socket.h"

// --- TEST PARA EL MENSAJE DE LOGIN ---
TEST(ProtocolTest, LoginSerializationIsSymmetric) {
    LoginDTO original_dto;
    original_dto.username = "jugador1";
    original_dto.password = "mi_password_secreta";

    Socket acceptor_skt("8080");
    Socket client_skt("localhost", "8080");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // El cliente envia
    client_protocol.send_login(original_dto);

    // El servidor recibe y procesa el OpCode automáticamente
    CommandVariant received_cmd = server_protocol.receive_command();

    // ASSERT: Verificamos que el variant contiene la alternativa correcta
    ASSERT_TRUE(std::holds_alternative<LoginDTO>(received_cmd));

    // Extraemos el DTO limpio y comparamos
    LoginDTO received_dto = std::get<LoginDTO>(received_cmd);
    EXPECT_EQ(original_dto.username, received_dto.username);
}

// --- TEST PARA EL MENSAJE DE START_MOVE ---
TEST(ProtocolTest, StartMoveSerializationIsSymmetric) {
    StartMoveDTO original_dto;
    original_dto.direction = static_cast<uint8_t>(Movement::UP);

    Socket acceptor_skt("8081");
    Socket client_skt("localhost", "8081");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // Enviamos usando el struct nuevo
    client_protocol.send_start_move(original_dto);

    CommandVariant received_cmd = server_protocol.receive_command();

    // ASSERT
    ASSERT_TRUE(std::holds_alternative<StartMoveDTO>(received_cmd));
    StartMoveDTO received_dto = std::get<StartMoveDTO>(received_cmd);
    
    EXPECT_EQ(original_dto.direction, received_dto.direction);
}

// --- TEST EXTRA PARA VALIDAR NUMEROS DE 16 BITS (htons/ntohs) ---
TEST(ProtocolTest, DropItemSerializationIsSymmetric) {
    DropItemDTO original_dto;
    original_dto.slot = 5;
    original_dto.amount = 1500; // Valor mayor a 255 para asegurar que usa 2 bytes

    Socket acceptor_skt("8082");
    Socket client_skt("localhost", "8082");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_drop_item(original_dto);

    CommandVariant received_cmd = server_protocol.receive_command();

    ASSERT_TRUE(std::holds_alternative<DropItemDTO>(received_cmd));
    DropItemDTO received_dto = std::get<DropItemDTO>(received_cmd);
    
    EXPECT_EQ(original_dto.slot, received_dto.slot);
    EXPECT_EQ(original_dto.amount, received_dto.amount);
}
