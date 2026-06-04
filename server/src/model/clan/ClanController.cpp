#include "ClanController.h"

#include <sstream>

void ClanController::dispatch(uint32_t senderDbId, const ClanCommandDTO& cmd,
                              const IWorldContext& worldCtx,
                              std::vector<ClanNotification>& outNotifs) {
    uint32_t targetDbId = 0;

    // Resolvemos el Nick a ID de forma anticipada solo para los comandos que apuntan a otro jugador
    if (cmd.type == ClanCommandType::ACCEPT || cmd.type == ClanCommandType::REJECT ||
        cmd.type == ClanCommandType::BAN || cmd.type == ClanCommandType::KICK) {

        targetDbId = cmd.targetDbId != 0 ? cmd.targetDbId : worldCtx.resolveNickToDbId(cmd.arg1);
        if (targetDbId == 0) {
            outNotifs.push_back({senderDbId, "Jugador '" + cmd.arg1 + "' no encontrado online."});
            return;
        }
    }

    // Ruteo
    switch (cmd.type) {
        case ClanCommandType::FOUND:
            handleFoundClan(senderDbId, worldCtx.getPlayerLevel(senderDbId), cmd.arg1, outNotifs);
            break;
        case ClanCommandType::JOIN:
            handleJoinRequest(senderDbId, cmd.arg1, outNotifs);
            break;
        case ClanCommandType::LEAVE:
            handleLeaveClan(senderDbId, outNotifs);
            break;
        case ClanCommandType::REVIEW:
            handleReviewClan(senderDbId, worldCtx, outNotifs);
            break;
        case ClanCommandType::ACCEPT:
            handleAcceptMember(senderDbId, targetDbId, cmd.arg1, outNotifs);
            break;
        case ClanCommandType::REJECT:
            handleRejectMember(senderDbId, targetDbId, cmd.arg1, outNotifs);
            break;
        case ClanCommandType::BAN:
            handleBanMember(senderDbId, targetDbId, cmd.arg1, outNotifs);
            break;
        case ClanCommandType::KICK:
            handleKickMember(senderDbId, targetDbId, cmd.arg1, outNotifs);
            break;
    }
}

void ClanController::broadcastToClan(const Clan* clan, const std::string& message,
                                     uint32_t excludeDbId,
                                     std::vector<ClanNotification>& outNotifs) const {
    if (!clan)
        return;
    for (uint32_t memberId: clan->getMembers()) {
        if (memberId != excludeDbId) {
            outNotifs.push_back({memberId, "[Clan] " + message});
        }
    }
}

void ClanController::handleFoundClan(uint32_t senderDbId, uint16_t senderLevel,
                                     const std::string& clanName,
                                     std::vector<ClanNotification>& outNotifs) {
    ClanOpResult result = service.foundClan(senderDbId, senderLevel, clanName);

    switch (result) {
        case ClanOpResult::ALREADY_IN_CLAN:
            outNotifs.push_back({senderDbId, "Ya perteneces a un clan y no puedes fundar otro."});
            break;
        case ClanOpResult::LEVEL_TOO_LOW:
            outNotifs.push_back({senderDbId, "Necesitas más nivel para fundar un clan."});
            break;
        case ClanOpResult::NAME_TAKEN:
            outNotifs.push_back(
                    {senderDbId, "El nombre de clan '" + clanName + "' ya está en uso."});
            break;
        case ClanOpResult::OK:
            outNotifs.push_back(
                    {senderDbId, "¡Felicitaciones! Fundaste el clan '" + clanName + "'."});
            break;
        default:
            break;
    }
}

void ClanController::handleJoinRequest(uint32_t senderDbId, const std::string& clanName,
                                       std::vector<ClanNotification>& outNotifs) {
    ClanOpResult result = service.joinRequest(senderDbId, clanName);

    if (result == ClanOpResult::ALREADY_IN_CLAN) {
        outNotifs.push_back({senderDbId, "Ya perteneces a un clan."});
    } else if (result == ClanOpResult::CLAN_NOT_FOUND) {
        outNotifs.push_back({senderDbId, "No existe un clan llamado '" + clanName + "'."});
    } else if (result == ClanOpResult::PLAYER_BANNED) {
        outNotifs.push_back({senderDbId, "Fuiste baneado de este clan. Solicitud rechazada."});
    } else if (result == ClanOpResult::CLAN_FULL) {
        outNotifs.push_back({senderDbId, "El clan '" + clanName + "' está lleno."});
    } else if (result == ClanOpResult::OK) {
        const Clan* clan = service.getClanByName(clanName);
        outNotifs.push_back(
                {senderDbId, "Solicitud enviada al clan '" + clanName + "'. Espera respuesta."});
        outNotifs.push_back({clan->getFounderDbId(),
                             "Nuevo pedido de ingreso al clan (ID: " + std::to_string(senderDbId) +
                                     "). Usa /revisar-clan."});
    }
}

void ClanController::handleAcceptMember(uint32_t senderDbId, uint32_t targetDbId,
                                        const std::string& targetNick,
                                        std::vector<ClanNotification>& outNotifs) {
    const Clan* clan = service.getClanOfPlayer(senderDbId);
    std::string clanName = clan ? clan->getName() : "";

    ClanOpResult result = service.acceptMember(senderDbId, targetDbId);

    if (result == ClanOpResult::NOT_IN_CLAN) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
    } else if (result == ClanOpResult::NOT_FOUNDER) {
        outNotifs.push_back({senderDbId, "Solo el fundador puede aceptar miembros."});
    } else if (result == ClanOpResult::NO_PENDING_REQUEST) {
        outNotifs.push_back({senderDbId, targetNick + " no tiene solicitud pendiente."});
    } else if (result == ClanOpResult::CLAN_FULL) {
        outNotifs.push_back({senderDbId, "El clan ya alcanzó el máximo de miembros."});
    } else if (result == ClanOpResult::OK) {
        outNotifs.push_back({targetDbId, "¡Fuiste aceptado en el clan '" + clanName + "'!"});
        broadcastToClan(service.getClanOfPlayer(senderDbId), targetNick + " se unió al clan.",
                        targetDbId, outNotifs);
    }
}

void ClanController::handleRejectMember(uint32_t senderDbId, uint32_t targetDbId,
                                        const std::string& targetNick,
                                        std::vector<ClanNotification>& outNotifs) {
    const Clan* clan = service.getClanOfPlayer(senderDbId);
    std::string clanName = clan ? clan->getName() : "";

    ClanOpResult result = service.rejectMember(senderDbId, targetDbId);

    if (result == ClanOpResult::NOT_IN_CLAN) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
    } else if (result == ClanOpResult::NOT_FOUNDER) {
        outNotifs.push_back({senderDbId, "Solo el fundador puede rechazar miembros."});
    } else if (result == ClanOpResult::NO_PENDING_REQUEST) {
        outNotifs.push_back({senderDbId, targetNick + " no tiene solicitud pendiente."});
    } else if (result == ClanOpResult::OK) {
        outNotifs.push_back(
                {targetDbId, "Tu solicitud para unirte a '" + clanName + "' fue rechazada."});
    }
}

void ClanController::handleBanMember(uint32_t senderDbId, uint32_t targetDbId,
                                     const std::string& targetNick,
                                     std::vector<ClanNotification>& outNotifs) {
    const Clan* clan = service.getClanOfPlayer(senderDbId);
    std::string clanName = clan ? clan->getName() : "";

    ClanOpResult result = service.banMember(senderDbId, targetDbId);

    if (result == ClanOpResult::NOT_IN_CLAN) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
    } else if (result == ClanOpResult::NOT_FOUNDER) {
        outNotifs.push_back({senderDbId, "Solo el fundador puede banear."});
    } else if (result == ClanOpResult::OK) {
        outNotifs.push_back({targetDbId, "Fuiste baneado del clan '" + clanName + "'."});
        broadcastToClan(clan, targetNick + " fue baneado del clan.", targetDbId, outNotifs);
    }
}

void ClanController::handleKickMember(uint32_t senderDbId, uint32_t targetDbId,
                                      const std::string& targetNick,
                                      std::vector<ClanNotification>& outNotifs) {
    const Clan* clan = service.getClanOfPlayer(senderDbId);
    std::string clanName = clan ? clan->getName() : "";

    ClanOpResult result = service.kickMember(senderDbId, targetDbId);

    if (result == ClanOpResult::NOT_IN_CLAN) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
    } else if (result == ClanOpResult::NOT_FOUNDER) {
        outNotifs.push_back({senderDbId, "No tienes permisos o intentaste auto-expulsarte."});
    } else if (result == ClanOpResult::PLAYER_NOT_FOUND) {
        outNotifs.push_back({senderDbId, targetNick + " no es miembro del clan."});
    } else if (result == ClanOpResult::OK) {
        outNotifs.push_back({targetDbId, "Fuiste expulsado del clan '" + clanName + "'."});
        broadcastToClan(clan, targetNick + " fue expulsado del clan.", targetDbId, outNotifs);
    }
}

void ClanController::handleLeaveClan(uint32_t senderDbId,
                                     std::vector<ClanNotification>& outNotifs) {
    const Clan* clan = service.getClanOfPlayer(senderDbId);
    std::string clanName = clan ? clan->getName() : "";

    ClanOpResult result = service.leaveClan(senderDbId);

    if (result == ClanOpResult::NOT_IN_CLAN) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
    } else if (result == ClanOpResult::CANNOT_LEAVE_AS_FOUNDER) {
        outNotifs.push_back({senderDbId, "El fundador no puede abandonar el clan."});
    } else if (result == ClanOpResult::OK) {
        outNotifs.push_back({senderDbId, "Abandonaste el clan '" + clanName + "'."});
        broadcastToClan(clan, "Un miembro abandonó el clan.", senderDbId, outNotifs);
    }
}

void ClanController::handleReviewClan(uint32_t senderDbId, const IWorldContext& worldCtx,
                                      std::vector<ClanNotification>& outNotifs) {

    const Clan* clan = service.getClanOfPlayer(senderDbId);

    if (!clan) {
        outNotifs.push_back({senderDbId, "No perteneces a ningún clan."});
        return;
    }
    if (clan->getFounderDbId() != senderDbId) {
        outNotifs.push_back({senderDbId, "Solo el fundador puede usar este comando."});
        return;
    }

    outNotifs.push_back({senderDbId, "=== Info del Clan ==="});

    outNotifs.push_back({senderDbId, "Miembros activos:"});

    for (uint32_t memberDbId: clan->getMembers()) {
        std::string name = worldCtx.getPlayerUsername(memberDbId).value_or("Desconocido");
        if (memberDbId == clan->getFounderDbId()) {
            outNotifs.push_back({senderDbId, " - " + name + " (Líder)"});
        } else {
            outNotifs.push_back({senderDbId, " - " + name});
        }
    }

    outNotifs.push_back({senderDbId, "Peticiones pendientes:"});
    const auto& requests = clan->getJoinRequests();

    if (requests.empty()) {
        outNotifs.push_back({senderDbId, " - (Ninguna)"});
    } else {
        for (uint32_t reqDbId: requests) {
            std::string name = worldCtx.getPlayerUsername(reqDbId).value_or("Desconocido");
            outNotifs.push_back({senderDbId, " - " + name});
        }
    }
}
