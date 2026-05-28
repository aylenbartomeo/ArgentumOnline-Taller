#ifndef CLAN_H
#define CLAN_H

#include <cstdint>
#include <string>
#include <unordered_set>

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
    Clan(uint32_t id, const std::string& name, uint32_t founderDbId) 
        : id(id), name(name), founderDbId(founderDbId) {
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
};

#endif