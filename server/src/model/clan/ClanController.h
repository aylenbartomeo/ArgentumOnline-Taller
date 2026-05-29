#ifndef CLAN_CONTROLLER_H
#define CLAN_CONTROLLER_H

#include <sstream>
#include <string>
#include <vector>

#include "../../common/include/dto/ClanCommandDTO.h"

#include "ClanService.h"
#include "IWorldContext.h"

// Mismo struct de notificaciones que usabas
struct ClanNotification {
    uint32_t targetDbId;
    std::string message;
};

class ClanController {
private:
    ClanService& service;

    // Genera mensajes a todos los miembros del clan (excepto excludeDbId) y los agrega a outNotifs
    void broadcastToClan(const Clan* clan, const std::string& message, uint32_t excludeDbId,
                         std::vector<ClanNotification>& outNotifs) const;

public:
    explicit ClanController(ClanService& service): service(service) {}

    // Procesa un comando de clan y lo enruta al handler correspondiente.
    // worldCtx se usa para obtener datos del mundo (nivel de jugador, resolver nicks, etc.)
    void dispatch(uint32_t senderDbId, const ClanCommandDTO& cmd, const IWorldContext& worldCtx,
                  std::vector<ClanNotification>& outNotifs);

    void handleFoundClan(uint32_t senderDbId, uint16_t senderLevel, const std::string& clanName,
                         std::vector<ClanNotification>& outNotifs);
    void handleJoinRequest(uint32_t senderDbId, const std::string& clanName,
                           std::vector<ClanNotification>& outNotifs);
    void handleAcceptMember(uint32_t senderDbId, uint32_t targetDbId, const std::string& targetNick,
                            std::vector<ClanNotification>& outNotifs);
    void handleRejectMember(uint32_t senderDbId, uint32_t targetDbId, const std::string& targetNick,
                            std::vector<ClanNotification>& outNotifs);
    void handleBanMember(uint32_t senderDbId, uint32_t targetDbId, const std::string& targetNick,
                         std::vector<ClanNotification>& outNotifs);
    void handleKickMember(uint32_t senderDbId, uint32_t targetDbId, const std::string& targetNick,
                          std::vector<ClanNotification>& outNotifs);
    void handleLeaveClan(uint32_t senderDbId, std::vector<ClanNotification>& outNotifs);
    void handleReviewClan(uint32_t senderDbId, std::vector<ClanNotification>& outNotifs);
};

#endif
