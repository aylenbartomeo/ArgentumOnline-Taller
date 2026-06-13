#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../config/MonsterConfig.h"
#include "../interfaces/Attackable.h"
#include "../interfaces/Interactable.h"

#include "Monster.h"
#include "Player.h"

class EntityManager {
private:
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;
    std::unordered_map<uint32_t, std::unique_ptr<Monster>> monsters;
    std::unordered_map<uint32_t, std::unique_ptr<Interactable>> cityNPCs;

    uint32_t nextEntityId = 1;
    std::unordered_map<uint32_t, uint32_t> dbIdToEntityId;

public:
    EntityManager() = default;
    ~EntityManager() = default;

    // --- Mapeo de IDs ---
    uint32_t allocateEntityId() { return nextEntityId++; }
    uint32_t resolveEntityId(uint32_t dbId) const;

    // --- Lookup Seguro (Elimina Boilerplate) ---
    Player* getPlayer(uint32_t dbId);
    const Player* getPlayer(uint32_t dbId) const;

    // Búsqueda polimórfica (por EntityId)
    Attackable* findAttackable(uint32_t entityId);
    Interactable* findInteractable(uint32_t entityId) const;

    // --- CRUD Players ---
    void registerPlayer(uint32_t entityId, uint32_t dbId, std::unique_ptr<Player> player);
    bool removePlayer(uint32_t dbId);

    // --- CRUD Monsters ---
    uint32_t addMonster(NPCType type, Position pos, const MonsterConfig& config);
    void addMonster(std::unique_ptr<Monster> monster);
    void eraseMonster(uint32_t entityId);

    // --- CRUD NPCs ---
    void addNPC(std::unique_ptr<Interactable> npc);

    // --- Consultas Rápidas ---
    std::vector<uint32_t> getOnlinePlayerDbIds() const;
    std::optional<Position> getPlayerPosition(uint32_t dbId) const;
    std::optional<std::string> getPlayerUsername(uint32_t dbId) const;
    uint16_t getPlayerLevel(uint32_t dbId) const;
    uint32_t resolveNickToDbId(const std::string& nick) const;

    size_t getPlayerCount() const;
    size_t getMonsterCount() const;
    bool isEmpty() const;

    // --- Acceso a Colecciones (Para Snapshot/Update) ---
    auto& getPlayers() { return players; }
    const auto& getPlayers() const { return players; }
    auto& getMonsters() { return monsters; }
    const auto& getMonsters() const { return monsters; }
    const auto& getCityNPCs() const { return cityNPCs; }
};

#endif  // ENTITY_MANAGER_H
