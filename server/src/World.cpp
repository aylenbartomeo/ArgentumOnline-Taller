#include "World.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>

#include "model/combat/CombatManager.h"
#include "model/entities/NPCFactory.h"
#include "model/entities/Player.h"

World::World(int worldId, const std::string& creatorPlayerName, const ItemRegistry& itemRegistry):
        worldId(worldId),
        creatorPlayerName(creatorPlayerName),
        itemRegistry(itemRegistry),
        map(),
        clanService(clanRepo),
        clanController(clanService) {
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

    // Notificar a clanmates antes de remover
    auto clanIdOpt = clanRepo.getClanIdOfPlayer(dbId);
    if (clanIdOpt) {
        const Clan* clan = clanRepo.getClanById(*clanIdOpt);
        auto pit = players.find(entityId);
        if (clan && pit != players.end()) {
            std::string playerName = pit->second->getName();
            for (uint32_t memberId: clan->getMembers()) {
                if (memberId != dbId) {
                    outgoingEvents.push_back(
                            {memberId, "[Clan] " + playerName + " salió del juego."});
                }
            }
        }
    }
    this->players.erase(entityId);
    this->dbIdToEntityId.erase(itMap);
    return true;
}

bool World::loadMap(const std::string& path) {
    if (map.loadSpawnFromJson(path)) {
        spawnNPCs();
        return true;
    }
    return false;
}

void World::spawnNPCs() {
    NPCFactory factory(itemRegistry, globalBank);
    for (const auto& spawn: map.getAllNPCs()) {
        uint32_t entityId = nextEntityId++;
        if (auto npc = factory.create(entityId, spawn.type, spawn.position)) {
            cityNPCs[entityId] = std::move(npc);
        }
    }
}

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

void World::playerAttack(uint32_t attackerDbId, uint32_t targetDbId) {
    auto mapIt = this->dbIdToEntityId.find(attackerDbId);
    if (mapIt == this->dbIdToEntityId.end())
        return;

    // El atacante siempre es un Player (viene de un comando del cliente)
    auto itAttacker = this->players.find(mapIt->second);
    if (itAttacker == this->players.end())
        return;

    Player& attacker = *(itAttacker->second);

    uint32_t targetEntityId = targetDbId;
    auto targetMapIt = this->dbIdToEntityId.find(targetDbId);
    if (targetMapIt != this->dbIdToEntityId.end()) {
        targetEntityId = targetMapIt->second;
    }

    // El target puede ser Player o Monster (IDs globalmente únicos)
    Attackable* target = findAttackable(targetEntityId);
    if (!target)
        return;

    // --- Validar zona segura ---
    if (map.isSafeZone(attacker.getPosition().x, attacker.getPosition().y) ||
        map.isSafeZone(target->getPosition().x, target->getPosition().y)) {
        outgoingEvents.push_back({attackerDbId, "You can't fight in a safe zone."});
        return;
    }

    if (areClanmates(attackerDbId, targetDbId)) {
        outgoingEvents.push_back({attackerDbId, "No puedes atacar a un miembro de tu clan."});
        return;
    }

    // --- Validar que el atacante pueda atacar ---
    if (!attacker.canAttack()) {
        outgoingEvents.push_back({attackerDbId, "You can't attack right now."});
        return;
    }

    // --- Validar linea de vision ---
    if (!map.hasLineOfSight(attacker.getPosition(), target->getPosition())) {
        outgoingEvents.push_back({attackerDbId, "There is an obstacle blocking your vision."});
        return;
    }

    if (enforceFairPlay &&
        (!attacker.canEngageInCombatWith(*target) || !target->canEngageInCombatWith(attacker))) {
        outgoingEvents.push_back(
                {attackerDbId, "You can't fight this target (fair play violation)."});
        return;
    }

    attacker.onActionStarted();

    // Notificar a los clanmates del target que está siendo atacado
    auto targetPlayerIt = dynamic_cast<Player*>(target);
    if (targetPlayerIt) {
        uint32_t targetDb = targetPlayerIt->getDbId();
        auto clanIdOpt = clanRepo.getClanIdOfPlayer(targetDb);
        if (clanIdOpt) {
            const Clan* clan = clanRepo.getClanById(*clanIdOpt);
            if (clan) {
                std::string alertMsg = "[Clan] " + targetPlayerIt->getName() +
                                       " está siendo atacado por " + attacker.getName() + "!";
                for (uint32_t memberId: clan->getMembers()) {
                    if (memberId != targetDb && memberId != attackerDbId) {
                        outgoingEvents.push_back({memberId, alertMsg});
                    }
                }
            }
        }
    }

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

            if (pTarget->isDead()) {
                handlePlayerDeath(pTarget->getDbId());
            }
        }
    }
}

// --- IA de Monstruos ---

void World::monsterAttack(const Monster& monster, Player& target) {

    // Validar zona segura
    if (map.isSafeZone(monster.getPosition().x, monster.getPosition().y) ||
        map.isSafeZone(target.getPosition().x, target.getPosition().y)) {
        return;
    }

    // Validar linea de visión
    if (!map.hasLineOfSight(monster.getPosition(), target.getPosition())) {
        return;
    }

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
        if (target.isDead()) {
            handlePlayerDeath(target.getDbId());
        }
    }
}

void World::playerInteract(uint32_t dbId, uint32_t targetNpcId) {
    // 1. Conseguir el ID interno de la entidad jugador
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end())
        return;
    uint32_t playerEntityId = itMap->second;

    auto itPlayer = this->players.find(playerEntityId);
    if (itPlayer == this->players.end())
        return;
    Player& player = *(itPlayer->second);

    // 2. Buscar al NPC
    Interactable* npc = this->findInteractable(targetNpcId);
    if (!npc)
        return;

    // 3. Validación de distancia Chebyshev (rango máximo 2 celdas)
    if (player.getPosition().chebyshev_distance_to(npc->getPosition()) > 2) {
        outgoingEvents.push_back({dbId, "El NPC está demasiado lejos."});
        return;
    }

    // 4. REGISTRO DE SESIÓN: El mundo toma nota de la interacción
    activeInteractions[playerEntityId] = npc;

    // 5. Polimorfismo: El NPC reacciona (pendiente acople con cliente/UI)
    npc->beInteractedBy(player);
}

void World::playerExecuteNpcCommand(uint32_t dbId, const NpcCommandDTO& dto) {
    auto itMap = this->dbIdToEntityId.find(dbId);
    if (itMap == this->dbIdToEntityId.end())
        return;
    uint32_t playerEntityId = itMap->second;

    auto itPlayer = this->players.find(playerEntityId);
    if (itPlayer == this->players.end())
        return;
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
        activeInteractions.erase(playerEntityId);  // Rompemos la sesión
        outgoingEvents.push_back({dbId, "Te has alejado demasiado del NPC."});
        return;
    }

    // El NPC ejecuta el comando de negocio (compra, venta, etc.)
    // Nota: el envío de WorldEvent hacia la UI está pendiente de acople
    npc->handleCommand(player, dto);
}

Player* World::findNearestPlayer(const Monster& monster, int range) {
    Player* nearest = nullptr;
    int minDist = range + 1;

    for (auto& [id, player]: players) {
        if (player->isDead())
            continue;

        if (map.isSafeZone(player->getPosition().x, player->getPosition().y))
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
    // Agregamos monstruos
    for (const auto& pair: monsters) {
        uint32_t id = pair.first;
        const Monster* monster = pair.second.get();
        snapshot.monsters.emplace_back(id, EntityType::MONSTER, monster->getPosition().x,
                                       monster->getPosition().y, monster->getHp(),
                                       monster->getMaxHp(), monster->getSpriteId());
    }

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
        snapshot.players.push_back(entityData);
    }

    // Items del suelo
    for (const auto& pair: map.getGroundItemsSnapshot()) {
        const Position& pos = pair.first;
        const GroundItem& item = pair.second;
        snapshot.groundItems.push_back(GroundItemDTO(item.itemId, item.amount, pos.x, pos.y));
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

bool World::placeItemOnGround(const Position& pos, uint32_t itemId, uint16_t amount) {
    return map.placeItem(pos, itemId, amount);
}

std::optional<Position> World::placeItemNearby(const Position& pos, uint32_t itemId,
                                               uint16_t amount) {
    return map.placeItemNearby(pos, itemId, amount);
}

std::optional<GroundItem> World::pickUpItemFromGround(const Position& pos) {
    return map.pickUpItem(pos);
}

bool World::isSafeZone(float x, float y) const { return map.isSafeZone(x, y); }

std::vector<WorldEvent> World::pollEvents() {
    std::vector<WorldEvent> events = std::move(outgoingEvents);
    outgoingEvents.clear();
    return events;
}

void World::pickUpItem(uint32_t dbId) {
    auto posOpt = getPlayerPosition(dbId);
    if (!posOpt)
        return;

    auto itemOpt = map.pickUpItem(posOpt.value());
    if (!itemOpt) {
        outgoingEvents.push_back({dbId, "There are no items here to pick up."});
        return;
    }

    auto itPlayer = players.find(dbIdToEntityId[dbId]);
    Player& player = *(itPlayer->second);

    uint16_t leftover = player.addInventoryItem(itemOpt->itemId, itemOpt->amount);

    if (leftover > 0) {
        outgoingEvents.push_back({dbId, "Inventory full. You couldn't pick up everything."});
        map.placeItem(posOpt.value(), itemOpt->itemId, leftover);
    } else {
        outgoingEvents.push_back({dbId, "Item picked up."});
    }
}

void World::dropItem(uint32_t dbId, uint8_t slot, uint16_t amount) {
    auto posOpt = getPlayerPosition(dbId);
    if (!posOpt)
        return;

    auto itPlayer = players.find(dbIdToEntityId[dbId]);
    Player& player = *(itPlayer->second);

    auto slotOpt = player.inspectInventorySlot(slot);
    if (!slotOpt || slotOpt->amount < amount)
        return;

    auto placedPos = map.placeItemNearby(posOpt.value(), slotOpt->item_id, amount);
    if (!placedPos) {
        outgoingEvents.push_back({dbId, "Not enough space on the ground to drop the item."});
        return;
    }

    player.removeInventoryItem(slot, amount);
}

void World::handlePlayerDeath(uint32_t dbId) {
    auto itMap = dbIdToEntityId.find(dbId);
    if (itMap == dbIdToEntityId.end())
        return;

    auto itPlayer = players.find(itMap->second);
    Player& player = *(itPlayer->second);
    Position pos = player.getPosition();

    uint32_t dropped_gold = player.dropExcessGold();
    if (dropped_gold > 0) {
        // TODO: map.placeItemNearby(pos, GOLD_ITEM_ID, dropped_gold);
    }

    std::vector<Slot> dropped_items = player.dropAllItems();
    for (const auto& slot: dropped_items) {
        map.placeItemNearby(pos, slot.item_id, slot.amount);
    }
}

// =============================================================================
// Implementación de IWorldContext para interactuar con clanes
// =============================================================================

uint16_t World::getPlayerLevel(uint32_t dbId) const {
    auto itMap = dbIdToEntityId.find(dbId);
    if (itMap == dbIdToEntityId.end())
        return 0;
    auto pit = players.find(itMap->second);
    if (pit == players.end())
        return 0;

    return pit->second->getLevel();
}

uint32_t World::resolveNickToDbId(const std::string& nick) const {
    for (const auto& [dbId, entityId]: dbIdToEntityId) {
        auto it = players.find(entityId);
        if (it != players.end() && it->second->getName() == nick) {
            return dbId;
        }
    }
    return 0;
}

// =============================================================================
// Sistema de clanes
// =============================================================================

void World::processClanCommand(uint32_t senderDbId, const ClanCommandDTO& cmd) {
    std::vector<ClanNotification> notifs;

    // Delega TODA la lógica de ruteo y ensamblado de strings al Controlador.
    clanController.dispatch(senderDbId, cmd, *this, notifs);

    // Volcar las notificaciones resultantes a los eventos de salida
    std::transform(notifs.begin(), notifs.end(), std::back_inserter(outgoingEvents),
                   [](const ClanNotification& n) {
                       return WorldEvent{n.targetDbId, n.message};
                   });
}

int World::countNearbyClanmates(uint32_t playerDbId, int range) const {
    auto clanIdOpt = clanRepo.getClanIdOfPlayer(playerDbId);
    if (!clanIdOpt)
        return 0;

    const Clan* clan = const_cast<ClanRepository&>(clanRepo).getClanById(*clanIdOpt);
    if (!clan)
        return 0;

    auto selfIt = dbIdToEntityId.find(playerDbId);
    if (selfIt == dbIdToEntityId.end())
        return 0;
    auto playerIt = players.find(selfIt->second);
    if (playerIt == players.end())
        return 0;
    Position selfPos = playerIt->second->getPosition();

    int count = 0;
    for (uint32_t memberId: clan->getMembers()) {
        if (memberId == playerDbId)
            continue;
        auto mapIt = dbIdToEntityId.find(memberId);
        if (mapIt == dbIdToEntityId.end())
            continue;
        auto pIt = players.find(mapIt->second);
        if (pIt == players.end())
            continue;

        Position p = pIt->second->getPosition();
        int dx = std::abs(p.x - selfPos.x);
        int dy = std::abs(p.y - selfPos.y);
        if (dx + dy <= range)
            count++;
    }
    return count;
}

bool World::areClanmates(uint32_t playerADbId, uint32_t playerBDbId) const {
    auto clanA = clanRepo.getClanIdOfPlayer(playerADbId);
    auto clanB = clanRepo.getClanIdOfPlayer(playerBDbId);
    return (clanA && clanB && *clanA == *clanB);
}
