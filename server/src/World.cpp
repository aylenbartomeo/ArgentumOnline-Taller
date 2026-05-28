#include "World.h"

#include <iostream>
#include <utility>

#include "model/combat/CombatManager.h"
#include "model/entities/Player.h"

World::World(int worldId, const std::string& creatorPlayerName, const ItemRegistry& itemRegistry):
        worldId(worldId), creatorPlayerName(creatorPlayerName), itemRegistry(itemRegistry), map() {
    map.setDimensions(20, 15);
    map.setSpawnPoint(0, 0);
}

std::string World::getCreatorPlayerName() const { return this->creatorPlayerName; }

int World::getWorldId() const { return this->worldId; }

bool World::addPlayer(uint32_t dbId, std::string& username,
                      const std::optional<Position>& savedPosition) {
    if (this->dbIdToEntityId.find(dbId) != this->dbIdToEntityId.end()) {
        return false;
    }

    uint32_t entityId = nextEntityId++;
    this->dbIdToEntityId[dbId] = entityId;

    PlayerConfig baseConfig = {15, 15, 15, 15, 1, 0, 0};
    RaceConfig raceConfig = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig classConfig = {1.0f, 1.0f, 1.0f, false};

    Position spawnPos;
    if (savedPosition.has_value() && map.canMoveTo(savedPosition.value())) {
        spawnPos = savedPosition.value();
    } else {
        std::pair<float, float> spawn = map.getInitialPosition();
        spawnPos = Position{static_cast<int>(spawn.first), static_cast<int>(spawn.second)};
    }
    this->players[entityId] = std::make_unique<Player>(
            entityId, dbId, username, raceConfig, classConfig, baseConfig, itemRegistry, spawnPos);

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
    if (itMap == this->dbIdToEntityId.end())
        return;

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
    if (mapIt == this->dbIdToEntityId.end())
        return;

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

    if (!res.attackHappened)
        return;

    if (res.evaded) {
        outgoingEvents.push_back(
                {attackerDbId, "The target (" + target->getName() + ") evaded your attack."});
        const Player* pTarget = dynamic_cast<const Player*>(target);
        if (pTarget) {
            outgoingEvents.push_back(
                    {pTarget->getDbId(), "You evaded the attack from " + attacker.getName() + "!"});
        }
    } else {
        std::string critMsg = res.critical ? " CRITICAL HIT!" : "";
        outgoingEvents.push_back({attackerDbId, "You dealt " + std::to_string(res.damage) +
                                                        " damage to " + target->getName() + "!" +
                                                        critMsg});
        const Player* pTarget = dynamic_cast<const Player*>(target);
        if (pTarget) {
            outgoingEvents.push_back(
                    {pTarget->getDbId(), "You received " + std::to_string(res.damage) +
                                                 " damage from " + attacker.getName() + "!"});
        }
    }
}

// --- IA de Monstruos ---

void World::monsterAttack(const Monster& monster, Player& target) {
    CombatResult res = CombatManager::getInstance().processAttack(monster, target);

    if (!res.attackHappened)
        return;

    if (res.evaded) {
        outgoingEvents.push_back(
                {target.getDbId(), "You evaded the attack from " + monster.getName() + "!"});
    } else {
        outgoingEvents.push_back({target.getDbId(), "You received " + std::to_string(res.damage) +
                                                            " damage from " + monster.getName() +
                                                            "!"});
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

std::optional<Position> World::getPlayerPosition(uint32_t dbId) const {
    auto itMap = dbIdToEntityId.find(dbId);
    if (itMap == dbIdToEntityId.end())
        return std::nullopt;
    auto it = players.find(itMap->second);
    if (it == players.end())
        return std::nullopt;
    return it->second->getPosition();
}

std::optional<std::string> World::getPlayerUsername(uint32_t dbId) const {
    auto itMap = dbIdToEntityId.find(dbId);
    if (itMap == dbIdToEntityId.end())
        return std::nullopt;
    auto it = players.find(itMap->second);
    if (it == players.end())
        return std::nullopt;
    return it->second->getName();
}

std::vector<uint32_t> World::getOnlinePlayerDbIds() const {
    std::vector<uint32_t> ids;
    ids.reserve(dbIdToEntityId.size());
    for (const auto& [dbId, entityId]: dbIdToEntityId) {
        ids.push_back(dbId);
    }
    return ids;
}

std::pair<float, float> World::getInitialPosition() { return map.getInitialPosition(); }

void World::setObstacleAt(int x, int y) { map.setObstacleInGrid(x, y, true); }

std::vector<WorldEvent> World::pollEvents() {
    std::vector<WorldEvent> events = std::move(outgoingEvents);
    outgoingEvents.clear();
    return events;
}

// =============================================================================
// Sistema de clanes
// =============================================================================
 
std::optional<uint32_t> World::resolveNickToDbId(const std::unordered_map<uint32_t, uint32_t>& dbIdToEntityId,
        const std::unordered_map<uint32_t, std::unique_ptr<Player>>& players, const std::string& nick) {
    for (const auto& [dbId, entityId] : dbIdToEntityId) {
        auto it = players.find(entityId);
        if (it != players.end() && it->second->getName() == nick) {
            return dbId;
        }
    }
    return std::nullopt;
}

void World::processClanCommand(uint32_t senderDbId, const ClanCommandDTO& cmd) {
    std::vector<ClanNotification> notifs;
 
    // Para comandos que requieren nivel, lo obtenemos del Player
    auto getLevel = [&]() -> uint16_t {
        auto it = dbIdToEntityId.find(senderDbId);
        if (it == dbIdToEntityId.end()) return 0;
        auto pit = players.find(it->second);
        if (pit == players.end()) return 0;
        return pit->second->getLevel();
    };
 
    // Resolver nick a dbId (para comandos que apuntan a otro jugador)
    auto resolveTarget = [&]() -> uint32_t {
        if (cmd.targetDbId != 0) return cmd.targetDbId;  // ya resuelto
        auto resolved = resolveNickToDbId(dbIdToEntityId, players, cmd.arg1);
        return resolved.value_or(0);
    };
 
    ClanOpResult result = ClanOpResult::OK;
 
    switch (cmd.type) {
        case ClanCommandType::FOUND:
            result = clanManager.foundClan(senderDbId, getLevel(), cmd.arg1, notifs);
            break;
 
        case ClanCommandType::JOIN:
            result = clanManager.joinRequest(senderDbId, cmd.arg1, notifs);
            break;
 
        case ClanCommandType::LEAVE:
            result = clanManager.leaveClan(senderDbId, notifs);
            break;
 
        case ClanCommandType::REVIEW: {
            std::string report;
            result = clanManager.reviewClan(senderDbId, report);
            if (result == ClanOpResult::OK) {
                notifs.push_back({senderDbId, report});
            } else {
                notifs.push_back({senderDbId, "No eres fundador de ningún clan."});
            }
            break;
        }
 
        case ClanCommandType::ACCEPT: {
            uint32_t targetId = resolveTarget();
            if (targetId == 0) {
                notifs.push_back({senderDbId, "Jugador '" + cmd.arg1 + "' no encontrado online."});
            } else {
                result = clanManager.acceptMember(senderDbId, cmd.arg1, targetId, notifs);
            }
            break;
        }
 
        case ClanCommandType::REJECT: {
            uint32_t targetId = resolveTarget();
            if (targetId == 0) {
                notifs.push_back({senderDbId, "Jugador '" + cmd.arg1 + "' no encontrado online."});
            } else {
                result = clanManager.rejectMember(senderDbId, cmd.arg1, targetId, notifs);
            }
            break;
        }
 
        case ClanCommandType::BAN: {
            uint32_t targetId = resolveTarget();
            if (targetId == 0) {
                notifs.push_back({senderDbId, "Jugador '" + cmd.arg1 + "' no encontrado online."});
            } else {
                result = clanManager.banMember(senderDbId, cmd.arg1, targetId, notifs);
            }
            break;
        }
 
        case ClanCommandType::KICK: {
            uint32_t targetId = resolveTarget();
            if (targetId == 0) {
                notifs.push_back({senderDbId, "Jugador '" + cmd.arg1 + "' no encontrado online."});
            } else {
                result = clanManager.kickMember(senderDbId, cmd.arg1, targetId, notifs);
            }
            break;
        }
    }
 
    // Volcar las notificaciones a los outgoingEvents del mundo
    for (const auto& n : notifs) {
        outgoingEvents.push_back({n.targetDbId, n.message});
    }
}
  
int World::countNearbyClanmates(uint32_t playerDbId, int range) const {
    // Construir mapa de posiciones de los miembros online
    std::unordered_map<uint32_t, std::pair<int,int>> memberPositions;
    for (const auto& [dbId, entityId] : dbIdToEntityId) {
        auto it = players.find(entityId);
        if (it != players.end()) {
            Position p = it->second->getPosition();
            memberPositions[dbId] = {p.x, p.y};
        }
    }
 
    // Posición de referencia: el propio jugador
    auto selfIt = dbIdToEntityId.find(playerDbId);
    if (selfIt == dbIdToEntityId.end()) return 0;
    auto playerIt = players.find(selfIt->second);
    if (playerIt == players.end()) return 0;
    Position selfPos = playerIt->second->getPosition();
 
    return clanManager.countNearbyClanmates(playerDbId, memberPositions, selfPos.x, selfPos.y, range);
}
 
bool World::areClanmates(uint32_t playerADbId, uint32_t playerBDbId) const {
    return clanManager.areClanmates(playerADbId, playerBDbId);
}
