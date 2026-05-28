#include "ClanManager.h"

#include <algorithm>
#include <cctype>
#include <sstream>

std::string ClanManager::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

Clan* ClanManager::getClanOfPlayer(uint32_t playerDbId) {
    auto it = playerClanIndex.find(playerDbId);
    if (it == playerClanIndex.end()) return nullptr;
    auto clanIt = clans.find(it->second);
    if (clanIt == clans.end()) return nullptr;
    return &clanIt->second;
}

const Clan* ClanManager::getClanOfPlayer(uint32_t playerDbId) const {
    auto it = playerClanIndex.find(playerDbId);
    if (it == playerClanIndex.end()) return nullptr;
    auto clanIt = clans.find(it->second);
    if (clanIt == clans.end()) return nullptr;
    return &clanIt->second;
}

Clan* ClanManager::findClanByName(const std::string& name) {
    auto it = nameIndex.find(toLower(name));
    if (it == nameIndex.end()) return nullptr;
    auto clanIt = clans.find(it->second);
    if (clanIt == clans.end()) return nullptr;
    return &clanIt->second;
}

void ClanManager::broadcastToClan(const Clan& clan, const std::string& message, uint32_t excludeDbId,
                                   std::vector<ClanNotification>& outNotifs) const {
    for (uint32_t memberId : clan.members) {
        if (memberId != excludeDbId) {
            outNotifs.push_back({memberId, "[Clan] " + message});
        }
    }
}

// ===========================================================================
// Operaciones de clan
// ===========================================================================

ClanOpResult ClanManager::foundClan(uint32_t founderDbId, uint16_t founderLevel, const std::string& clanName,
                                    std::vector<ClanNotification>& outNotifs) {
    // El fundador no puede ya pertenecer a un clan
    if (playerClanIndex.count(founderDbId)) {
        outNotifs.push_back({founderDbId, "Ya perteneces a un clan y no puedes fundar otro."});
        return ClanOpResult::ALREADY_IN_CLAN;
    }

    // Nivel mínimo
    if (founderLevel < CLAN_MIN_LEVEL_TO_FOUND) {
        outNotifs.push_back({founderDbId,
            "Necesitas nivel " + std::to_string(CLAN_MIN_LEVEL_TO_FOUND) +
            " para fundar un clan (tu nivel: " + std::to_string(founderLevel) + ")."});
        return ClanOpResult::LEVEL_TOO_LOW;
    }

    // Nombre único
    if (nameIndex.count(toLower(clanName))) {
        outNotifs.push_back({founderDbId,
            "El nombre de clan '" + clanName + "' ya está en uso."});
        return ClanOpResult::NAME_TAKEN;
    }

    // Crear clan
    uint32_t clanId = nextClanId++;
    Clan& clan = clans[clanId];
    clan.id          = clanId;
    clan.name        = clanName;
    clan.founderDbId = founderDbId;
    clan.members.insert(founderDbId);

    nameIndex[toLower(clanName)]   = clanId;
    playerClanIndex[founderDbId]   = clanId;

    outNotifs.push_back({founderDbId,
        "¡Felicitaciones! Fundaste el clan '" + clanName + "'."});
    return ClanOpResult::OK;
}

ClanOpResult ClanManager::joinRequest(uint32_t playerDbId, const std::string& clanName,
                                    std::vector<ClanNotification>& outNotifs) {
    if (playerClanIndex.count(playerDbId)) {
        outNotifs.push_back({playerDbId, "Ya perteneces a un clan."});
        return ClanOpResult::ALREADY_IN_CLAN;
    }

    Clan* clan = findClanByName(clanName);
    if (!clan) {
        outNotifs.push_back({playerDbId, "No existe un clan llamado '" + clanName + "'."});
        return ClanOpResult::CLAN_NOT_FOUND;
    }

    // Verificar ban
    if (clan->banned.count(playerDbId)) {
        outNotifs.push_back({playerDbId,
            "Fuiste baneado del clan '" + clanName + "'. Tu solicitud fue rechazada automáticamente."});
        return ClanOpResult::PLAYER_BANNED;
    }

    // Verificar capacidad (contamos miembros + solicitudes ya aceptadas = solo miembros)
    if (clan->members.size() >= CLAN_MAX_MEMBERS) {
        outNotifs.push_back({playerDbId, "El clan '" + clanName + "' está lleno."});
        return ClanOpResult::CLAN_FULL;
    }

    clan->pendingRequests.insert(playerDbId);

    outNotifs.push_back({playerDbId,
        "Solicitud enviada al clan '" + clan->name + "'. Espera la respuesta del fundador."});
    // Notificar al fundador
    outNotifs.push_back({clan->founderDbId,
        "Nuevo pedido de ingreso al clan de parte de un jugador (id=" +
        std::to_string(playerDbId) + "). Usa /revisar-clan para verlo."});

    return ClanOpResult::OK;
}

ClanOpResult ClanManager::acceptMember(uint32_t founderDbId, const std::string& targetNick, uint32_t targetDbId,
                                        std::vector<ClanNotification>& outNotifs) {
    Clan* clan = getClanOfPlayer(founderDbId);
    if (!clan) {
        outNotifs.push_back({founderDbId, "No perteneces a ningún clan."});
        return ClanOpResult::NOT_IN_CLAN;
    }

    if (clan->founderDbId != founderDbId) {
        outNotifs.push_back({founderDbId, "Solo el fundador puede aceptar miembros."});
        return ClanOpResult::NOT_FOUNDER;
    }

    if (!clan->pendingRequests.count(targetDbId)) {
        outNotifs.push_back({founderDbId,
            targetNick + " no tiene una solicitud pendiente."});
        return ClanOpResult::NO_PENDING_REQUEST;
    }

    if (clan->members.size() >= CLAN_MAX_MEMBERS) {
        outNotifs.push_back({founderDbId, "El clan ya alcanzó el máximo de miembros."});
        return ClanOpResult::CLAN_FULL;
    }

    clan->pendingRequests.erase(targetDbId);
    clan->members.insert(targetDbId);
    playerClanIndex[targetDbId] = clan->id;

    outNotifs.push_back({targetDbId,
        "¡Fuiste aceptado en el clan '" + clan->name + "'!"});
    broadcastToClan(*clan, targetNick + " se unió al clan.", targetDbId, outNotifs);

    return ClanOpResult::OK;
}

ClanOpResult ClanManager::rejectMember(uint32_t founderDbId, const std::string& targetNick, uint32_t targetDbId,
                                        std::vector<ClanNotification>& outNotifs) {
    Clan* clan = getClanOfPlayer(founderDbId);
    if (!clan) {
        outNotifs.push_back({founderDbId, "No perteneces a ningún clan."});
        return ClanOpResult::NOT_IN_CLAN;
    }
    if (clan->founderDbId != founderDbId) {
        outNotifs.push_back({founderDbId, "Solo el fundador puede rechazar miembros."});
        return ClanOpResult::NOT_FOUNDER;
    }
    if (!clan->pendingRequests.count(targetDbId)) {
        outNotifs.push_back({founderDbId, targetNick + " no tiene solicitud pendiente."});
        return ClanOpResult::NO_PENDING_REQUEST;
    }

    clan->pendingRequests.erase(targetDbId);
    outNotifs.push_back({targetDbId,
        "Tu solicitud para unirte al clan '" + clan->name + "' fue rechazada."});

    return ClanOpResult::OK;
}

ClanOpResult ClanManager::banMember(uint32_t founderDbId, const std::string& targetNick, uint32_t targetDbId,
                                    std::vector<ClanNotification>& outNotifs) {
    Clan* clan = getClanOfPlayer(founderDbId);
    if (!clan) {
        outNotifs.push_back({founderDbId, "No perteneces a ningún clan."});
        return ClanOpResult::NOT_IN_CLAN;
    }
    if (clan->founderDbId != founderDbId) {
        outNotifs.push_back({founderDbId, "Solo el fundador puede banear."});
        return ClanOpResult::NOT_FOUNDER;
    }

    // Puede venir de pendingRequests o de members (si ya estaba adentro)
    clan->pendingRequests.erase(targetDbId);
    if (clan->members.count(targetDbId)) {
        clan->members.erase(targetDbId);
        playerClanIndex.erase(targetDbId);
    }
    clan->banned.insert(targetDbId);

    outNotifs.push_back({targetDbId,
        "Fuiste baneado del clan '" + clan->name + "'."});
    broadcastToClan(*clan, targetNick + " fue baneado del clan.", targetDbId, outNotifs);
    return ClanOpResult::OK;
}

ClanOpResult ClanManager::kickMember(uint32_t founderDbId, const std::string& targetNick, uint32_t targetDbId,
                                    std::vector<ClanNotification>& outNotifs) {
    Clan* clan = getClanOfPlayer(founderDbId);
    if (!clan) {
        outNotifs.push_back({founderDbId, "No perteneces a ningún clan."});
        return ClanOpResult::NOT_IN_CLAN;
    }
    if (clan->founderDbId != founderDbId) {
        outNotifs.push_back({founderDbId, "Solo el fundador puede expulsar miembros."});
        return ClanOpResult::NOT_FOUNDER;
    }
    if (targetDbId == founderDbId) {
        outNotifs.push_back({founderDbId, "No puedes expulsarte a ti mismo."});
        return ClanOpResult::NOT_FOUNDER;
    }
    if (!clan->members.count(targetDbId)) {
        outNotifs.push_back({founderDbId, targetNick + " no es miembro del clan."});
        return ClanOpResult::PLAYER_NOT_FOUND;
    }

    clan->members.erase(targetDbId);
    playerClanIndex.erase(targetDbId);

    outNotifs.push_back({targetDbId,
        "Fuiste expulsado del clan '" + clan->name + "'."});
    broadcastToClan(*clan, targetNick + " fue expulsado del clan.", targetDbId, outNotifs);

    return ClanOpResult::OK;
}

ClanOpResult ClanManager::leaveClan(uint32_t playerDbId, std::vector<ClanNotification>& outNotifs) {
    Clan* clan = getClanOfPlayer(playerDbId);
    if (!clan) {
        outNotifs.push_back({playerDbId, "No perteneces a ningún clan."});
        return ClanOpResult::NOT_IN_CLAN;
    }
    if (clan->founderDbId == playerDbId) {
        outNotifs.push_back({playerDbId,
            "El fundador no puede abandonar el clan."});
        return ClanOpResult::CANNOT_LEAVE_AS_FOUNDER;
    }

    std::string clanName = clan->name;
    clan->members.erase(playerDbId);
    playerClanIndex.erase(playerDbId);

    outNotifs.push_back({playerDbId,
        "Abandonaste el clan '" + clanName + "'."});
    broadcastToClan(*clan, "Un miembro abandonó el clan.", playerDbId, outNotifs);

    return ClanOpResult::OK;
}

ClanOpResult ClanManager::reviewClan(uint32_t founderDbId, std::string& outReport) {
    const Clan* clan = getClanOfPlayer(founderDbId);
    if (!clan) return ClanOpResult::NOT_IN_CLAN;
    if (clan->founderDbId != founderDbId) return ClanOpResult::NOT_FOUNDER;

    std::ostringstream ss;
    ss << "=== Clan: " << clan->name << " ===\n";
    ss << "Miembros (" << clan->members.size() << "/" << CLAN_MAX_MEMBERS << "):\n";
    for (uint32_t id : clan->members) {
        ss << "  - id=" << id;
        if (id == clan->founderDbId) ss << " [Fundador]";
        ss << "\n";
    }
    ss << "Solicitudes pendientes (" << clan->pendingRequests.size() << "):\n";
    for (uint32_t id : clan->pendingRequests) {
        ss << "  - id=" << id << "\n";
    }
    ss << "Baneados (" << clan->banned.size() << "):\n";
    for (uint32_t id : clan->banned) {
        ss << "  - id=" << id << "\n";
    }

    outReport = ss.str();
    return ClanOpResult::OK;
}

// ==========================================================================
// Consultas
// ==========================================================================

std::optional<uint32_t> ClanManager::getClanId(uint32_t playerDbId) const {
    auto it = playerClanIndex.find(playerDbId);
    if (it == playerClanIndex.end()) return std::nullopt;
    return it->second;
}

bool ClanManager::areClanmates(uint32_t playerADbId, uint32_t playerBDbId) const {
    auto itA = playerClanIndex.find(playerADbId);
    auto itB = playerClanIndex.find(playerBDbId);
    if (itA == playerClanIndex.end() || itB == playerClanIndex.end()) return false;
    return itA->second == itB->second;
}

int ClanManager::countNearbyClanmates(
        uint32_t playerDbId,
        const std::unordered_map<uint32_t, std::pair<int,int>>& memberPositions,
        int refX, int refY, int range) const {

    const Clan* clan = getClanOfPlayer(playerDbId);
    if (!clan) return 0;

    int count = 0;
    for (uint32_t memberId : clan->members) {
        if (memberId == playerDbId) continue;   // no contar a sí mismo
        auto it = memberPositions.find(memberId);
        if (it == memberPositions.end()) continue;  // offline
        int dx = std::abs(it->second.first  - refX);
        int dy = std::abs(it->second.second - refY);
        if (dx + dy <= range) ++count;
    }
    return count;
}

// ==========================================================================
// Notificaciones de login/logoff
// ==========================================================================

void ClanManager::notifyLogin(uint32_t playerDbId, const std::string& username,
                               std::vector<ClanNotification>& outNotifs) const {
    const Clan* clan = getClanOfPlayer(playerDbId);
    if (!clan) return;
    for (uint32_t memberId : clan->members) {
        if (memberId != playerDbId) {
            outNotifs.push_back({memberId,
                "[Clan] " + username + " entró a Argentum."});
        }
    }
}

void ClanManager::notifyLogoff(uint32_t playerDbId, const std::string& username,
                                std::vector<ClanNotification>& outNotifs) const {
    const Clan* clan = getClanOfPlayer(playerDbId);
    if (!clan) return;
    for (uint32_t memberId : clan->members) {
        if (memberId != playerDbId) {
            outNotifs.push_back({memberId,
                "[Clan] " + username + " salió de Argentum."});
        }
    }
}

void ClanManager::notifyUnderAttack(uint32_t attackedDbId, const std::string& attackedName,
                                    std::vector<ClanNotification>& outNotifs) const {
    const Clan* clan = getClanOfPlayer(attackedDbId);
    if (!clan) return;
    for (uint32_t memberId : clan->members) {
        if (memberId != attackedDbId) {
            outNotifs.push_back({memberId,
                "[Clan] ¡" + attackedName + " está siendo atacado!"});
        }
    }
}