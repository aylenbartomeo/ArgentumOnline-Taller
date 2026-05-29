#include "ClanService.h"

ClanOpResult ClanService::foundClan(uint32_t founderDbId, uint16_t founderLevel,
                                    const std::string& clanName) {
    if (repo.getClanIdOfPlayer(founderDbId))
        return ClanOpResult::ALREADY_IN_CLAN;
    if (founderLevel < minLevelToFound)
        return ClanOpResult::LEVEL_TOO_LOW;
    if (repo.isNameTaken(clanName))
        return ClanOpResult::NAME_TAKEN;

    repo.createClan(clanName, founderDbId);
    return ClanOpResult::OK;
}

ClanOpResult ClanService::joinRequest(uint32_t playerDbId, const std::string& clanName) {
    if (repo.getClanIdOfPlayer(playerDbId))
        return ClanOpResult::ALREADY_IN_CLAN;

    Clan* clan = repo.getClanByName(clanName);
    if (!clan)
        return ClanOpResult::CLAN_NOT_FOUND;
    if (clan->isBanned(playerDbId))
        return ClanOpResult::PLAYER_BANNED;
    if (clan->isFull())
        return ClanOpResult::CLAN_FULL;

    clan->addPendingRequest(playerDbId);
    return ClanOpResult::OK;
}

ClanOpResult ClanService::acceptMember(uint32_t founderDbId, uint32_t targetDbId) {
    Clan* clan = repo.getClanOfPlayer(founderDbId);
    if (!clan)
        return ClanOpResult::NOT_IN_CLAN;
    if (clan->getFounderDbId() != founderDbId)
        return ClanOpResult::NOT_FOUNDER;
    if (!clan->hasPendingRequest(targetDbId))
        return ClanOpResult::NO_PENDING_REQUEST;
    if (clan->isFull())
        return ClanOpResult::CLAN_FULL;

    clan->removePendingRequest(targetDbId);
    repo.addPlayerToClan(targetDbId, clan->getId());
    return ClanOpResult::OK;
}

ClanOpResult ClanService::rejectMember(uint32_t founderDbId, uint32_t targetDbId) {
    Clan* clan = repo.getClanOfPlayer(founderDbId);
    if (!clan)
        return ClanOpResult::NOT_IN_CLAN;
    if (clan->getFounderDbId() != founderDbId)
        return ClanOpResult::NOT_FOUNDER;
    if (!clan->hasPendingRequest(targetDbId))
        return ClanOpResult::NO_PENDING_REQUEST;

    clan->removePendingRequest(targetDbId);
    return ClanOpResult::OK;
}

ClanOpResult ClanService::banMember(uint32_t founderDbId, uint32_t targetDbId) {
    Clan* clan = repo.getClanOfPlayer(founderDbId);
    if (!clan)
        return ClanOpResult::NOT_IN_CLAN;
    if (clan->getFounderDbId() != founderDbId)
        return ClanOpResult::NOT_FOUNDER;

    clan->banPlayer(targetDbId);
    repo.removePlayerFromClan(targetDbId);  // En caso de que fuera miembro activo
    return ClanOpResult::OK;
}

ClanOpResult ClanService::kickMember(uint32_t founderDbId, uint32_t targetDbId) {
    const Clan* clan = repo.getClanOfPlayer(founderDbId);
    if (!clan)
        return ClanOpResult::NOT_IN_CLAN;
    if (clan->getFounderDbId() != founderDbId)
        return ClanOpResult::NOT_FOUNDER;
    if (targetDbId == founderDbId)
        return ClanOpResult::NOT_FOUNDER;  // No auto-kick
    if (!clan->isMember(targetDbId))
        return ClanOpResult::PLAYER_NOT_FOUND;

    repo.removePlayerFromClan(targetDbId);
    return ClanOpResult::OK;
}

ClanOpResult ClanService::leaveClan(uint32_t playerDbId) {
    const Clan* clan = repo.getClanOfPlayer(playerDbId);
    if (!clan)
        return ClanOpResult::NOT_IN_CLAN;
    if (clan->getFounderDbId() == playerDbId)
        return ClanOpResult::CANNOT_LEAVE_AS_FOUNDER;

    repo.removePlayerFromClan(playerDbId);
    return ClanOpResult::OK;
}
