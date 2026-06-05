#include "EntityManager.h"

uint32_t EntityManager::resolveEntityId(uint32_t dbId) const {
    auto it = dbIdToEntityId.find(dbId);
    if (it != dbIdToEntityId.end()) {
        return it->second;
    }
    return 0;
}

Player* EntityManager::getPlayer(uint32_t dbId) {
    uint32_t entityId = resolveEntityId(dbId);
    if (entityId == 0) return nullptr;

    auto it = players.find(entityId);
    if (it != players.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Player* EntityManager::getPlayer(uint32_t dbId) const {
    uint32_t entityId = resolveEntityId(dbId);
    if (entityId == 0) return nullptr;

    auto it = players.find(entityId);
    if (it != players.end()) {
        return it->second.get();
    }
    return nullptr;
}

Attackable* EntityManager::findAttackable(uint32_t entityId) {
    auto itPlayer = players.find(entityId);
    if (itPlayer != players.end()) {
        return itPlayer->second.get();
    }

    auto itMonster = monsters.find(entityId);
    if (itMonster != monsters.end()) {
        return itMonster->second.get();
    }
    return nullptr;
}

Interactable* EntityManager::findInteractable(uint32_t entityId) {
    auto itNpc = cityNPCs.find(entityId);
    if (itNpc != cityNPCs.end()) {
        return itNpc->second.get();
    }
    return nullptr;
}

void EntityManager::registerPlayer(uint32_t entityId, uint32_t dbId, std::unique_ptr<Player> player) {
    if (dbIdToEntityId.find(dbId) != dbIdToEntityId.end()) {
        return; // Ya existe
    }

    dbIdToEntityId[dbId] = entityId;
    players[entityId] = std::move(player);
}

bool EntityManager::removePlayer(uint32_t dbId) {
    uint32_t entityId = resolveEntityId(dbId);
    if (entityId == 0) return false;

    players.erase(entityId);
    dbIdToEntityId.erase(dbId);
    return true;
}

uint32_t EntityManager::addMonster(NPCType type, Position pos, const MonsterConfig& config) {
    uint32_t entityId = nextEntityId++;
    monsters[entityId] = std::make_unique<Monster>(entityId, type, pos, config);
    return entityId;
}

void EntityManager::addMonster(std::unique_ptr<Monster> monster) {
    if (!monster) return;
    uint32_t id = monster->getId();
    monsters[id] = std::move(monster);
    if (id >= nextEntityId) {
        nextEntityId = id + 1;
    }
}

void EntityManager::eraseMonster(uint32_t entityId) {
    monsters.erase(entityId);
}

void EntityManager::addNPC(std::unique_ptr<Interactable> npc) {
    if (!npc) return;
    // We expect the interactable to already have an ID or we don't care because cityNPC uses ID
    // Actually in the original code: 
    // cityNPCs[entityId] = std::make_unique<Merchant>(entityId, pos, itemRegistry);
    // So the entityId must match the map key. 
    // Usually NPCs are constructed with an ID. We should just map it.
    uint32_t id = npc->getId();
    cityNPCs[id] = std::move(npc);
}

std::vector<uint32_t> EntityManager::getOnlinePlayerDbIds() const {
    std::vector<uint32_t> dbIds;
    dbIds.reserve(dbIdToEntityId.size());
    for (const auto& pair : dbIdToEntityId) {
        dbIds.push_back(pair.first);
    }
    return dbIds;
}

std::optional<Position> EntityManager::getPlayerPosition(uint32_t dbId) const {
    const Player* p = getPlayer(dbId);
    if (p) {
        return p->getPosition();
    }
    return std::nullopt;
}

std::optional<std::string> EntityManager::getPlayerUsername(uint32_t dbId) const {
    const Player* p = getPlayer(dbId);
    if (p) {
        return p->getName();
    }
    return std::nullopt;
}

uint16_t EntityManager::getPlayerLevel(uint32_t dbId) const {
    const Player* p = getPlayer(dbId);
    if (p) {
        return p->getLevel();
    }
    return 0;
}

uint32_t EntityManager::resolveNickToDbId(const std::string& nick) const {
    for (const auto& pair : players) {
        if (pair.second->getName() == nick) {
            // Buscamos el dbId correspondiente
            for (const auto& mapPair : dbIdToEntityId) {
                if (mapPair.second == pair.first) {
                    return mapPair.first;
                }
            }
        }
    }
    return 0;
}

size_t EntityManager::getPlayerCount() const {
    return players.size();
}

size_t EntityManager::getMonsterCount() const {
    return monsters.size();
}

bool EntityManager::isEmpty() const {
    return players.empty();
}
