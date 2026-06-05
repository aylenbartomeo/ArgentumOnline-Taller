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

ClanRepositoryPersistData ClanRepository::toPersistData() const {
    ClanRepositoryPersistData data{};
    data.headers.reserve(clans.size());
    data.members.reserve(clans.size());
    data.pending.reserve(clans.size());
    data.banned.reserve(clans.size());
    for (const auto& pair: clans) {
        const auto& clan = pair.second;
        auto bundle = clan.toPersistData();
        data.headers.push_back(bundle.header);
        data.members.push_back(bundle.members);
        data.pending.push_back(bundle.pending);
        data.banned.push_back(bundle.banned);
    }
    return data;
}

void ClanRepository::fromPersistData(const ClanRepositoryPersistData& data) {
    uint32_t maxClanId = 0;
    for (size_t i = 0; i < data.headers.size(); ++i) {
        const auto& header = data.headers[i];
        if (header.clanId > maxClanId)
            maxClanId = header.clanId;

        clans.emplace(header.clanId, Clan(header.clanId, header.name, header.founderDbId));
        nameIndex[toLower(header.name)] = header.clanId;

        Clan& clan = clans.at(header.clanId);

        for (const auto& m: data.members[i]) {
            clan.addMember(m.dbId);
            playerClanIndex[m.dbId] = header.clanId;
        }
        for (const auto& p: data.pending[i]) {
            clan.addPendingRequest(p.dbId);
        }
        for (const auto& b: data.banned[i]) {
            clan.banPlayer(b.dbId);
        }
    }
    nextClanId = maxClanId + 1;
}
