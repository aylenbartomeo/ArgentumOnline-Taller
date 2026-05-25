#include "World.h"

#include <iostream>

#include "model/combat/CombatManager.h"
#include "model/entities/Player.h"

World::World(int worldId, const std::string& creatorPlayerName, const ItemRegistry& itemRegistry):
        worldId(worldId), creatorPlayerName(creatorPlayerName), itemRegistry(itemRegistry), map() {
    map.setDimensions(20, 15);
    map.setSpawnPoint(0, 0);
}

std::string World::getCreatorPlayerName() const { return this->creatorPlayerName; }

int World::getWorldId() const { return this->worldId; }

bool World::addPlayer(uint32_t dbId, std::string& username) {
    if (this->dbIdToEntityId.find(dbId) != this->dbIdToEntityId.end()) {
        return false;
    }

    uint32_t entityId = nextEntityId++;
    this->dbIdToEntityId[dbId] = entityId;

    PlayerConfig baseConfig = {15, 15, 15, 15, 1, 0, 0};
    RaceConfig raceConfig = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig classConfig = {1.0f, 1.0f, 1.0f, false};

    std::pair<float, float> spawn = map.getInitialPosition();
    this->players[entityId] = std::make_unique<Player>(
            entityId, dbId, username, raceConfig, classConfig, baseConfig, itemRegistry,
            Position{static_cast<int>(spawn.first), static_cast<int>(spawn.second)});

    return true;
}

bool World::removePlayer(uint32_t dbId) {
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end()) {
        return false;  // El jugador no pertenecía a este mundo
    }

    uint32_t entityId = itMap->second;
    this->players.erase(entityId);
    this->dbIdToEntityId.erase(itMap);
    return true;
}

bool World::loadMap(const std::string& path) { return map.loadSpawnFromJson(path); }

uint32_t World::addMonster(NPCType type, Position pos, const MonsterConfig& config) {
    uint32_t entityId = nextEntityId++;
    monsters[entityId] = std::make_unique<Monster>(entityId, type, pos, config);
    return entityId;
}

// --- Búsqueda polimórfica de entidades ---

Attackable* World::findAttackable(uint32_t id) {
    auto itPlayer = players.find(id);
    if (itPlayer != players.end()) {
        return itPlayer->second.get();
    }

    auto itMonster = monsters.find(id);
    if (itMonster != monsters.end()) {
        return itMonster->second.get();
    }

    return nullptr;
}

// --- Acciones del jugador ---

void World::moveEntity(uint32_t dbId, Movement direction) {
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end()) return;

    auto it = this->players.find(itMap->second);
    if (it == this->players.end())
        return;

    Player& player = *(it->second);
    player.onActionStarted();
    Position candidate = player.tryMove(direction);

    if (!map.canMoveTo(candidate))
        return;

    player.setPosition(candidate);
}

void World::playerAttack(uint32_t attackerDbId, uint32_t targetId) {
    auto mapIt = this->dbIdToEntityId.find(attackerDbId);
    if (mapIt == this->dbIdToEntityId.end()) return;

    // El atacante siempre es un Player (viene de un comando del cliente)
    auto itAttacker = this->players.find(mapIt->second);
    if (itAttacker == this->players.end())
        return;

    Player& attacker = *(itAttacker->second);

    // El target puede ser Player o Monster (IDs globalmente únicos)
    Attackable* target = findAttackable(targetId);
    if (!target)
        return;

    if (!attacker.canAttack()) {
        std::cout << "[WORLD] Attacker state prevents attacking." << std::endl;
        return;
    }

    if (!attacker.canEngageInCombatWith(*target) || !target->canEngageInCombatWith(attacker)) {
        std::cout << "[WORLD] Fair play rules prevent this combat." << std::endl;
        return;
    }

    attacker.onActionStarted();

    CombatResult res = CombatManager::getInstance().processAttack(attacker, *target);
    
    if (!res.attackHappened) return;

    if (res.evaded) {
        outgoingEvents.push_back({attackerDbId, "The target (" + target->getName() + ") evaded your attack."});
        Player* pTarget = dynamic_cast<Player*>(target);
        if (pTarget) {
            outgoingEvents.push_back({pTarget->getDbId(), "You evaded the attack from " + attacker.getName() + "!"});
        }
    } else {
        std::string critMsg = res.critical ? " CRITICAL HIT!" : "";
        outgoingEvents.push_back({attackerDbId, "You dealt " + std::to_string(res.damage) + " damage to " + target->getName() + "!" + critMsg});
        Player* pTarget = dynamic_cast<Player*>(target);
        if (pTarget) {
            outgoingEvents.push_back({pTarget->getDbId(), "You received " + std::to_string(res.damage) + " damage from " + attacker.getName() + "!"});
        }
    }
}

// --- IA de Monstruos ---

void World::monsterAttack(const Monster& monster, Player& target) {
    CombatResult res = CombatManager::getInstance().processAttack(monster, target);
    
    if (!res.attackHappened) return;

    if (res.evaded) {
        outgoingEvents.push_back({target.getDbId(), "You evaded the attack from " + monster.getName() + "!"});
    } else {
        outgoingEvents.push_back({target.getDbId(), "You received " + std::to_string(res.damage) + " damage from " + monster.getName() + "!"});
    }
}


Player* World::findNearestPlayer(const Monster& monster, int range) {
    Player* nearest = nullptr;
    int minDist = range + 1;

    for (auto& [id, player]: players) {
        if (player->isDead())
            continue;
        int dist = monster.distance_to(*player);
        if (dist <= range && dist < minDist) {
            minDist = dist;
            nearest = player.get();
        }
    }

    return nearest;
}

// --- Update del mundo (tick del game loop) ---

void World::update(float delta_time) {
    // Regeneración de Players
    for (auto& [id, player]: players) {
        player->update(delta_time);
    }

    // IA de Monstruos: detectar y atacar jugadores cercanos
    for (auto& [id, monster]: monsters) {
        if (monster->isDead())
            continue;

        // Buscar al Player más cercano dentro del rango de detección
        Player* target = findNearestPlayer(*monster, monster->get_detection_range());
        if (!target)
            continue;

        int dist = monster->distance_to(*target);

        if (dist <= monster->get_attack_range()) {
            // Está en rango de ataque: atacar
            monsterAttack(*monster, *target);
        }
        // Si está en rango de detección pero no de ataque, mover hacia el jugador...
    }
}

// --- Snapshot ---

SnapshotDTO World::generateSnapshot() const {
    SnapshotDTO snapshot;
    uint16_t spriteId = 1;
    for (const auto& pair: this->players) {
        const Player& player = *(pair.second);
        Position pos = player.getPosition();

        // Creamos el DTO de la entidad con datos mockeados/reales para el MVP
        EntityDTO entityData;
        entityData.id = player.getDbId();
        entityData.type = EntityType::PLAYER;
        entityData.x = pos.x;
        entityData.y = pos.y;
        entityData.current_hp = player.getHp();
        entityData.max_hp = player.getMaxHp();
        entityData.sprite_id = spriteId;  // Un ID de sprite por defecto para que el cliente dibuje
        spriteId++;  // Incrementamos el spriteId para que cada jugador tenga un sprite diferente
                     // (solo para demo)
        snapshot.entities.push_back(entityData);
    }

    return snapshot;
}

int World::getPlayerCount() const { return static_cast<int>(this->players.size()); }

bool World::isEmpty() const { return this->players.empty(); }

void World::setObstacleAt(int x, int y) { map.setObstacleInGrid(x, y, true); }

std::vector<WorldEvent> World::pollEvents() {
    std::vector<WorldEvent> events = std::move(outgoingEvents);
    outgoingEvents.clear();
    return events;
}
