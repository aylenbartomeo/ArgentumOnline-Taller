#ifndef CLAN_SERVICE_H
#define CLAN_SERVICE_H

#include "ClanRepository.h"
#include <string>

enum class ClanOpResult {
    OK, CLAN_NOT_FOUND, PLAYER_NOT_FOUND, ALREADY_IN_CLAN, NOT_IN_CLAN,
    NOT_FOUNDER, LEVEL_TOO_LOW, NAME_TAKEN, CLAN_FULL, PLAYER_BANNED,
    NO_PENDING_REQUEST, CANNOT_LEAVE_AS_FOUNDER, SAME_CLAN
};

class ClanService {
private:
    ClanRepository& repo;

public:
    explicit ClanService(ClanRepository& repo) : repo(repo) {}

    ClanOpResult foundClan(uint32_t founderDbId, uint16_t founderLevel, const std::string& clanName);
    ClanOpResult joinRequest(uint32_t playerDbId, const std::string& clanName);
    ClanOpResult acceptMember(uint32_t founderDbId, uint32_t targetDbId);
    ClanOpResult rejectMember(uint32_t founderDbId, uint32_t targetDbId);
    ClanOpResult banMember(uint32_t founderDbId, uint32_t targetDbId);
    ClanOpResult kickMember(uint32_t founderDbId, uint32_t targetDbId);
    ClanOpResult leaveClan(uint32_t playerDbId);
    
    Clan* getClanOfPlayer(uint32_t playerDbId) { return repo.getClanOfPlayer(playerDbId); }
    Clan* getClanByName(const std::string& clanName) { return repo.getClanByName(clanName); }
};

#endif