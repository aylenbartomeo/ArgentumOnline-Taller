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

    // NUEVO: Instanciamos un protocolo para cada lado de la comunicación
    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // 2. Act (Ejecutar)
    // El cliente usa su protocolo para enviar el DTO
    client_protocol.send_login(original_dto);
    
    // (Simulamos que el Receiver leyó el OpCode del socket servidor manualmente)
    uint8_t opcode;
    server_skt.recvall(&opcode, 1);
    
    // El servidor usa su protocolo para recibir y reconstruir el DTO
    LoginDTO received_dto = server_protocol.receive_login();

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

    // NUEVO: Instanciamos un protocolo para cada lado
    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // 2. Act
    client_protocol.send_start_move(original_dto);
    
    uint8_t opcode;
    server_skt.recvall(&opcode, 1);
    
    StartMoveDTO received_dto = server_protocol.receive_start_move();

    // 3. Assert
    EXPECT_EQ(opcode, static_cast<uint8_t>(OPCODE::START_MOVE));
    EXPECT_EQ(original_dto.direction, received_dto.direction);
}