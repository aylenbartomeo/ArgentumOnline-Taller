#ifndef CLAN_REPOSITORY_H
#define CLAN_REPOSITORY_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Clan.h"

class ClanRepository {
private:
    uint32_t nextClanId = 1;
    std::unordered_map<uint32_t, Clan> clans;
    std::unordered_map<std::string, uint32_t> nameIndex;
    std::unordered_map<uint32_t, uint32_t> playerClanIndex;

    static std::string toLower(const std::string& s);

public:
    Clan* createClan(const std::string& name, uint32_t founderDbId);

    Clan* getClanById(uint32_t clanId);
    Clan* getClanByName(const std::string& name);
    Clan* getClanOfPlayer(uint32_t playerDbId);

    std::optional<uint32_t> getClanIdOfPlayer(uint32_t playerDbId) const;

    void addPlayerToClan(uint32_t playerDbId, uint32_t clanId);
    void removePlayerFromClan(uint32_t playerDbId);

    bool isNameTaken(const std::string& name) const;

    // Para persistencia
    const std::unordered_map<uint32_t, Clan>& getAllClans() const { return clans; }
    void restoreClan(uint32_t clanId, const std::string& name, uint32_t founderDbId,
                     const std::vector<uint32_t>& memberDbIds,
                     const std::vector<uint32_t>& pendingDbIds,
                     const std::vector<uint32_t>& bannedDbIds);
    void setNextClanId(uint32_t id) { nextClanId = id; }
};

#endif
