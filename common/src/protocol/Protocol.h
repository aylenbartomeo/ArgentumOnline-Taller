#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

#include "../../include/OpCodes.h"
#include "../../include/dto/ClientCommands.h"
#include "../../include/dto/CreateCharacterDTO.h"
#include "../../include/dto/JoinResponseDTO.h"
#include "../../include/dto/LoginDTO.h"
#include "../../include/dto/LoginResponseDTO.h"
#include "../../include/dto/PlayerStatsDTO.h"
#include "../../include/dto/RegisterDTO.h"
#include "../../include/dto/StartMoveDTO.h"
#include "../socket/socket.h"
#include "dto/Snapshot.h"

class Protocol {
private:
    Socket& skt;

    void sendUint8(uint8_t value);
    void sendUint16(uint16_t value);
    void sendUint32(uint32_t value);

    void sendFloat(float value);
    float recvFloat();

    void sendString(const std::string& str);

    uint8_t recvUint8();
    uint16_t recvUint16();
    uint32_t recvUint32();

    std::string recvString();

public:
    explicit Protocol(Socket& skt);


    CommandVariant receive_command();

    uint8_t recv_opcode();

    // --- MÉTODOS DE ENVÍO (Cliente -> Servidor) ---
    void sendRegister(const RegisterDTO& dto);
    void sendLogin(const LoginDTO& loginDTO);
    void sendStartMove(const StartMoveDTO& dto);
    void sendStopMove();
    void sendAttack(uint32_t targetId);
    void sendSelectNpc(uint32_t targetId);
    void sendDropItem(const DropItemDTO& dto);
    void sendEquipItem(const EquipItemDTO& dto);
    void sendUseItem(const UseItemDTO& dto);
    void sendGrabItem();
    void sendChat(const ChatDTO& dto);
    void sendPrivateChat(const PrivateChatDTO& dto);
    void sendMeditate();
    void sendResurrect();
    void sendNpcCommand(const NpcCommandDTO& dto);
    void sendClanCommand(const ClanCommandDTO& dto);
    void sendCreateCharacter(const CreateCharacterDTO& dto);

    // --- MÉTODOS DE ENVÍO (Servidor -> Cliente) ---
    void sendSnapshot(const SnapshotDTO& snap);
    SnapshotDTO receiveSnapshotBody();

    void sendPlayerStats(const PlayerStatsDTO& stats);
    PlayerStatsDTO receivePlayerStatsBody();

    ChatDTO receiveChatBody();
    PrivateChatDTO receivePrivateChatBody();
    void receiveMeditateBody();
    void receiveResurrectBody();
    NpcCommandDTO receiveNpcCommandBody();
    ClanCommandDTO receiveClanCommandBody();

    void sendRegisterSuccess(uint32_t clientId);
    void sendRegisterFailed(const std::string& errorMessage);
    LoginResponseDTO recvRegisterResponse();
    void sendLoginSuccess(uint32_t clientId);
    void sendLoginFailed(const std::string& errorMessage);
    LoginResponseDTO recvLoginResponse();

    void sendJoinResponse(const JoinResponseDTO& dto);
    JoinResponseDTO receiveJoinResponseBody();

    // --- MÉTODO DE ENVÍO DE CHEATS ---
    void sendCheat(const CheatDTO& dto);

    // -- Metodos de proyectiles --
    void sendShoot(const ShootDTO& dto);
};

#endif
