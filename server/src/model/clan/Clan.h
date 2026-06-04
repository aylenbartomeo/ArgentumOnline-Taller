#ifndef CLAN_H
#define CLAN_H

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_set>
#include <vector>

#include "../../persistence/WorldPersistData.h"

struct ClanPersistDataBundle {
    ClanHeaderPersistData header;
    std::vector<ClanPlayerPersistData> members;
    std::vector<ClanPlayerPersistData> pending;
    std::vector<ClanPlayerPersistData> banned;
};

static constexpr uint16_t CLAN_MIN_LEVEL_TO_FOUND = 6;
static constexpr uint16_t CLAN_MAX_MEMBERS = 16;
static constexpr int CLAN_BONUS_RANGE = 10;
static constexpr float CLAN_DEFENSE_BONUS_PER_MEMBER = 0.05f;
static constexpr float CLAN_ATTACK_BONUS_PER_MEMBER = 0.05f;

class Clan {
private:
    uint32_t id;
    std::string name;
    uint32_t founderDbId;

    std::unordered_set<uint32_t> members;
    std::unordered_set<uint32_t> pendingRequests;
    std::unordered_set<uint32_t> banned;

public:
    Clan(uint32_t id, const std::string& name, uint32_t founderDbId):
            id(id), name(name), founderDbId(founderDbId) {
        members.insert(founderDbId);
    }

    uint32_t getId() const { return id; }
    const std::string& getName() const { return name; }
    uint32_t getFounderDbId() const { return founderDbId; }
    const std::unordered_set<uint32_t>& getMembers() const { return members; }
    const std::unordered_set<uint32_t>& getPendingRequests() const { return pendingRequests; }
    const std::unordered_set<uint32_t>& getBanned() const { return banned; }

    bool isFull() const { return members.size() >= CLAN_MAX_MEMBERS; }
    bool isMember(uint32_t dbId) const { return members.count(dbId) > 0; }
    bool isBanned(uint32_t dbId) const { return banned.count(dbId) > 0; }
    bool hasPendingRequest(uint32_t dbId) const { return pendingRequests.count(dbId) > 0; }

    void addMember(uint32_t dbId) { members.insert(dbId); }
    void removeMember(uint32_t dbId) { members.erase(dbId); }

    void addPendingRequest(uint32_t dbId) { pendingRequests.insert(dbId); }
    void removePendingRequest(uint32_t dbId) { pendingRequests.erase(dbId); }

    void banPlayer(uint32_t dbId) {
        banned.insert(dbId);
        removePendingRequest(dbId);
    }

    ClanPersistDataBundle toPersistData() const {
        ClanPersistDataBundle bundle{};
        bundle.header.clanId = id;
        bundle.header.founderDbId = founderDbId;
        std::strncpy(bundle.header.name, name.c_str(), sizeof(bundle.header.name) - 1);
        bundle.header.name[sizeof(bundle.header.name) - 1] = '\0';
        bundle.header.memberCount = members.size();
        bundle.header.pendingCount = pendingRequests.size();
        bundle.header.bannedCount = banned.size();

        bundle.members.reserve(members.size());
        std::transform(members.begin(), members.end(), std::back_inserter(bundle.members),
                       [](uint32_t memberId) { return ClanPlayerPersistData{memberId}; });

        bundle.pending.reserve(pendingRequests.size());
        std::transform(pendingRequests.begin(), pendingRequests.end(),
                       std::back_inserter(bundle.pending),
                       [](uint32_t pendingId) { return ClanPlayerPersistData{pendingId}; });

        bundle.banned.reserve(banned.size());
        std::transform(banned.begin(), banned.end(), std::back_inserter(bundle.banned),
                       [](uint32_t bannedId) { return ClanPlayerPersistData{bannedId}; });

        return bundle;
    }
};

#endif
