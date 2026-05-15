#include <gtest/gtest.h>
#include "../common/src/protocol/Protocol.h"
#include "../common/include/dto/LoginDTO.h"
#include "../common/include/dto/StartMoveDTO.h"
#include "../common/src/socket/socket.h"

// --- TEST PARA EL MENSAJE DE LOGIN ---
TEST(ProtocolTest, LoginSerializationIsSymmetric) {
    // 1. Arrange (Preparar)
    LoginDTO original_dto("jugador1", "mi_password_secreta");
    Socket acceptor_skt("8080");
    Socket client_skt("localhost", "8080");
    Socket server_skt = acceptor_skt.accept();

    // 2. Act (Ejecutar)
    // El cliente envía el DTO
    Protocol::send_login(original_dto, client_skt);
    
    // (Simulamos que el Receiver leyó el OpCode del socket servidor)
    uint8_t opcode;
    server_skt.recvall(&opcode, 1);
    
    // El servidor recibe y reconstruye el DTO
    LoginDTO received_dto = Protocol::receive_login(server_skt);

    // 3. Assert (Verificar)
    // Comprobamos que lo que salió del servidor sea idéntico a lo que entró por el cliente
    EXPECT_EQ(opcode, static_cast<uint8_t>(OPCODE::LOGIN));
    EXPECT_EQ(original_dto.username, received_dto.username);
    EXPECT_EQ(original_dto.password, received_dto.password);
}

// --- TEST PARA EL MENSAJE DE START_MOVE ---
TEST(ProtocolTest, StartMoveSerializationIsSymmetric) {
    // 1. Arrange
    StartMoveDTO original_dto(Direction::UP);
 
    Socket acceptor_skt("8081");
    Socket client_skt("localhost", "8081");
    Socket server_skt = acceptor_skt.accept();

    // 2. Act
    Protocol::send_start_move(original_dto, client_skt);
    uint8_t opcode;
    server_skt.recvall(&opcode, 1);
    StartMoveDTO received_dto = Protocol::receive_start_move(server_skt);

    // 3. Assert
    EXPECT_EQ(opcode, static_cast<uint8_t>(OPCODE::START_MOVE));
    EXPECT_EQ(original_dto.direction, received_dto.direction);
}