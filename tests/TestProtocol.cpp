#include <gtest/gtest.h>

#include "../common/include/dto/ClientCommands.h"
#include "../common/include/dto/LoginDTO.h"
#include "../common/include/dto/StartMoveDTO.h"
#include "../common/src/protocol/Protocol.h"
#include "../common/src/socket/socket.h"
#include "dto/CommandDTO.h"

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
    original_dto.direction = Movement::UP;

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
    original_dto.amount = 1500;  // Valor mayor a 255 para asegurar que usa 2 bytes

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

// --- TEST PARA EL MENSAJE DE STOP_MOVE (DTO Vacío) ---
TEST(ProtocolTest, StopMoveSerializationIsSymmetric) {
    Socket acceptor_skt("8083");
    Socket client_skt("localhost", "8083");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // El cliente envía el comando STOP_MOVE y el servidor lee del socket y reconstruye el comando
    client_protocol.send_stop_move();
    CommandVariant received_cmd = server_protocol.receive_command();

    // Se verifica que lo recibido sea efectivamente un StopMoveDTO
    ASSERT_TRUE(std::holds_alternative<StopMoveDTO>(received_cmd));
}

// --- TEST PARA EL MENSAJE DE ATTACK (DTO Vacío) ---
TEST(ProtocolTest, AttackSerializationIsSymmetric) {
    Socket acceptor_skt("8084");
    Socket client_skt("localhost", "8084");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_attack();

    CommandVariant received_cmd = server_protocol.receive_command();

    ASSERT_TRUE(std::holds_alternative<AttackDTO>(received_cmd));
}

// --- TEST PARA EQUIP ITEM (Payload: 1 byte) ---
TEST(ProtocolTest, EquipItemSerializationIsSymmetric) {
    EquipItemDTO original_dto;
    original_dto.slot = 3;

    Socket acceptor_skt("8085");
    Socket client_skt("localhost", "8085");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_equip_item(original_dto);

    CommandVariant received_cmd = server_protocol.receive_command();

    // Se valida que el tipo sea correcto
    ASSERT_TRUE(std::holds_alternative<EquipItemDTO>(received_cmd));
    EquipItemDTO received_dto =
            std::get<EquipItemDTO>(received_cmd);     // Se extrae el DTO del variant
    EXPECT_EQ(original_dto.slot, received_dto.slot);  // Se compara el contenido
}

// --- TEST PARA USE ITEM (Payload: 1 byte) ---
TEST(ProtocolTest, UseItemSerializationIsSymmetric) {
    UseItemDTO original_dto;
    original_dto.slot = 1;

    Socket acceptor_skt("8086");
    Socket client_skt("localhost", "8086");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_use_item(original_dto);

    CommandVariant received_cmd = server_protocol.receive_command();

    ASSERT_TRUE(std::holds_alternative<UseItemDTO>(received_cmd));  // Validación de tipo
    UseItemDTO received_dto = std::get<UseItemDTO>(received_cmd);   // Extracción del DTO
    EXPECT_EQ(original_dto.slot, received_dto.slot);                // Comparación de datos
}

// --- TEST PARA GRAB ITEM (DTO Vacío) ---
TEST(ProtocolTest, GrabItemSerializationIsSymmetric) {
    Socket acceptor_skt("8087");
    Socket client_skt("localhost", "8087");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_grab_item();

    CommandVariant received_cmd = server_protocol.receive_command();

    ASSERT_TRUE(std::holds_alternative<GrabItemDTO>(received_cmd));
}

// --- TEST PARA EL MENSAJE DE CHAT (Payload: string dinámico) ---
TEST(ProtocolTest, ChatSerializationIsSymmetric) {
    ChatDTO original_dto;
    original_dto.message = "Hola servidor, un gusto!";

    Socket acceptor_skt("8088");
    Socket client_skt("localhost", "8088");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    client_protocol.send_chat(original_dto);

    CommandVariant received_cmd = server_protocol.receive_command();  // Recepción y parseo

    ASSERT_TRUE(std::holds_alternative<ChatDTO>(received_cmd));
    ChatDTO received_dto = std::get<ChatDTO>(received_cmd);
    EXPECT_EQ(original_dto.message, received_dto.message);  // Comparación del contenido del string
}

// =================================================================
// --- TEST PARA EL VIAJE DE VUELTA: SERVIDOR -> CLIENTE (SNAPSHOT)
// =================================================================
TEST(ProtocolTest, SnapshotSerializationIsSymmetric) {
    // 1. Armamos un Snapshot simulado
    SnapshotDTO original_snap;

    EntityDTO entity1;
    entity1.id = 100;
    entity1.type = EntityType::PLAYER;
    entity1.x = 15;
    entity1.y = 20;
    entity1.current_hp = 50;
    entity1.max_hp = 100;

    EntityDTO entity2;
    entity2.id = 200;
    entity2.type = EntityType::MONSTER;
    entity2.x = 30;
    entity2.y = 40;
    entity2.current_hp = 10;
    entity2.max_hp = 20;

    original_snap.players.push_back(entity1);
    original_snap.monsters.push_back(entity2);

    Socket acceptor_skt("8089");
    Socket client_skt("localhost", "8089");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    // ACT: El SERVIDOR envía
    server_protocol.send_snapshot(original_snap);

    // ACT: El CLIENTE recibe
    uint8_t opcode = client_protocol.recv_opcode();
    EXPECT_EQ(opcode, static_cast<uint8_t>(OPCODE::SNAPSHOT));
    SnapshotDTO received_snap = client_protocol.receive_snapshot_body();

    // ASSERT: Validamos
    ASSERT_EQ(received_snap.players.size(), 1u);
    EXPECT_EQ(received_snap.players[0].id, 100);
    EXPECT_EQ(received_snap.players[0].type, EntityType::PLAYER);
    EXPECT_EQ(received_snap.players[0].x, 15);

    ASSERT_EQ(received_snap.monsters.size(), 1u);
    EXPECT_EQ(received_snap.monsters[0].id, 200);
    EXPECT_EQ(received_snap.monsters[0].type, EntityType::MONSTER);
}

// --- TESTS PARA LA RESPUESTA DE LOGIN (SERVIDOR -> CLIENTE) ---
TEST(ProtocolTest, LoginResponseSuccessIsSymmetric) {
    Socket acceptor_skt("8090");
    Socket client_skt("localhost", "8090");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    uint32_t expected_client_id = 42;

    // SERVER envia respuesta de exito
    server_protocol.send_login_success(expected_client_id);

    // CLIENT recibe y procesa respuesta
    LoginResponseDTO response = client_protocol.recv_login_response();

    // Validamos que la respuesta indique exito y tenga el ID correcto
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.clientId, expected_client_id);
    EXPECT_EQ(response.errorMessage, "");
}

TEST(ProtocolTest, LoginResponseFailureIsSymmetric) {
    Socket acceptor_skt("8091");
    Socket client_skt("localhost", "8091");
    Socket server_skt = acceptor_skt.accept();

    Protocol client_protocol(client_skt);
    Protocol server_protocol(server_skt);

    std::string expected_error = "Usuario o contraseña incorrecta";

    // SERVER rechaza login y envia un error
    server_protocol.send_login_failed(expected_error);

    // CLIENT recibe y procesa la respuesta
    LoginResponseDTO response = client_protocol.recv_login_response();

    // Validamos que la respuesta indique fallo y traiga el mensaje correcto
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.errorMessage, expected_error);
}
