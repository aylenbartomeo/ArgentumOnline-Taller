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
    // Si mientras interactua se va de la partida, se corta la interaccion con NPC
    activeInteractions.erase(entityId);
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

Interactable* World::findInteractable(uint32_t id) {
    auto itNpc = cityNPCs.find(id);
    if (itNpc != cityNPCs.end()) {
        return itNpc->second.get();
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

    // SI SE MOVIÓ, ya no esta interactuando
    activeInteractions.erase(itMap->second);
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

void World::playerInteract(uint32_t dbId, uint32_t targetNpcId) {
    // 1. Conseguir el ID interno de la entidad jugador
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end()) return;
    uint32_t playerEntityId = itMap->second;

    auto itPlayer = this->players.find(playerEntityId);
    if (itPlayer == this->players.end()) return;
    Player& player = *(itPlayer->second);

    // 2. Buscar al NPC
    Interactable* npc = this->findInteractable(targetNpcId); 
    if (!npc) return;

    // 3. Validación de distancia Chebyshev (rango máximo 2 celdas)
    if (player.getPosition().chebyshev_distance_to(npc->getPosition()) > 2) {
        outgoingEvents.push_back({dbId, "El NPC está demasiado lejos."});
        return;
    }

    // 4. REGISTRO DE SESIÓN: El mundo toma nota de la interacción
    activeInteractions[playerEntityId] = npc;

    // 5. Polimorfismo: El NPC reacciona (puede enviar un evento para abrir la GUI en el cliente)
    npc->beInteractedBy(player, outgoingEvents);
}

void World::playerExecuteNpcCommand(uint32_t dbId, const NpcCommandDTO& dto) {
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end()) return;
    uint32_t playerEntityId = itMap->second;

    auto itPlayer = this->players.find(playerEntityId);
    if (itPlayer == this->players.end()) return;
    Player& player = *(itPlayer->second);

    // El mundo busca al NPC en su tabla de interacciones, no en el Player
    auto itInteract = activeInteractions.find(playerEntityId);
    if (itInteract == activeInteractions.end()) {
        outgoingEvents.push_back({dbId, "Debes seleccionar un NPC primero."});
        return;
    }

    Interactable* npc = itInteract->second;

    // Validación de seguridad por si se movió mediante cheats o desincro
    if (player.getPosition().chebyshev_distance_to(npc->getPosition()) > 2) {
        activeInteractions.erase(playerEntityId); // Rompemos la sesión
        outgoingEvents.push_back({dbId, "Te has alejado demasiado del NPC."});
        return;
    }

    // El NPC ejecuta el comando de negocio (compra, venta, etc.)
    npc->handleCommand(player, dto, outgoingEvents);
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
