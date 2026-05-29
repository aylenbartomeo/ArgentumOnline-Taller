#include "ClanRepository.h"

#include <algorithm>
#include <cctype>

std::string ClanRepository::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

Clan* ClanRepository::createClan(const std::string& name, uint32_t founderDbId) {
    uint32_t clanId = nextClanId++;
    clans.emplace(clanId, Clan(clanId, name, founderDbId));

    nameIndex[toLower(name)] = clanId;
    playerClanIndex[founderDbId] = clanId;

    return &clans.at(clanId);
}

Clan* ClanRepository::getClanById(uint32_t clanId) {
    auto it = clans.find(clanId);
    return it != clans.end() ? &it->second : nullptr;
}

Clan* ClanRepository::getClanByName(const std::string& name) {
    auto it = nameIndex.find(toLower(name));
    return it != nameIndex.end() ? getClanById(it->second) : nullptr;
}

Clan* ClanRepository::getClanOfPlayer(uint32_t playerDbId) {
    auto idOpt = getClanIdOfPlayer(playerDbId);
    return idOpt ? getClanById(*idOpt) : nullptr;
}

std::optional<uint32_t> ClanRepository::getClanIdOfPlayer(uint32_t playerDbId) const {
    auto it = playerClanIndex.find(playerDbId);
    return it != playerClanIndex.end() ? std::optional<uint32_t>(it->second) : std::nullopt;
}

void ClanRepository::addPlayerToClan(uint32_t playerDbId, uint32_t clanId) {
    Clan* clan = getClanById(clanId);
    if (clan) {
        clan->addMember(playerDbId);
        playerClanIndex[playerDbId] = clanId;
    }
}

void ClanRepository::removePlayerFromClan(uint32_t playerDbId) {
    Clan* clan = getClanOfPlayer(playerDbId);
    if (clan) {
        clan->removeMember(playerDbId);
        playerClanIndex.erase(playerDbId);
    }
}

bool ClanRepository::isNameTaken(const std::string& name) const {
    return nameIndex.count(toLower(name)) > 0;
}
