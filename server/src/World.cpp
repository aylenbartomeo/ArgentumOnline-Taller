#include "World.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <random>

#include "../../common/include/dto/CheatDTO.h"
#include "config/MonsterConfigLoader.h"
#include "model/entities/Merchant.h"
#include "model/entities/NPCFactory.h"
#include "model/entities/Priest.h"
#include "model/items/ItemConstants.h"

#include "LootResolver.h"

World::World(int worldId, const std::string& creatorPlayerName, const ItemRegistry& itemRegistry,
             const CharacterConfigs& configs, const InventoryConfig& inventoryConfig,
             const ServerConfig& config):
        worldId(worldId),
        creatorPlayerName(creatorPlayerName),
        itemRegistry(itemRegistry),
        inventoryConfig(inventoryConfig),
        map(),
        clanService(clanRepo),
        clanController(clanService),
        characterConfigs(configs),
        combatSystem(map, entityManager, clanRepo, eventPublisher, *this, enforceFairPlay, config),
        projectileSystem(map, entityManager, combatSystem) {
    map.setDimensions(20, 15);
    map.setSpawnPoint(0, 0);
    try {
        std::string configPath = "config/monsters.toml";
        if (!std::filesystem::exists(configPath)) {
            configPath = "../config/monsters.toml";
        }
        MonsterConfigs mc = MonsterConfigLoader::loadMonsterConfigs(configPath);
        spawnSystem = SpawnSystem(std::move(mc), 5000.0f, 100);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] No se pudo cargar monsters.toml en el inicio: " << e.what()
                  << std::endl;
    }
}

void World::playerCheat(uint32_t dbId, CheatType type) {
    Player* player = entityManager.getPlayer(dbId);
    if (!player)
        return;

    if (type == CheatType::LEVEL_UP) {
        uint32_t needed = FormulaEngine::getInstance().calculate_level_up_limit(player->getLevel());
        player->addExperience(needed);
        eventPublisher.sendTo(dbId, "[CHEAT] ¡Has subido de nivel mágicamente!");
    } else if (type == CheatType::DIE) {
        if (!player->isDead()) {
            player->handleDeath();
            this->handlePlayerDeath(dbId);
            eventPublisher.sendTo(dbId, "[CHEAT] Te has suicidado.");
        } else {
            eventPublisher.sendTo(dbId, "[CHEAT] Ya estás muerto.");
        }
    } else if (type == CheatType::GIVE_RANGED_WEAPONS) {
        Player* p = entityManager.getPlayer(dbId);
        if (p) {
            p->addItem(2010, 1);
            p->addItem(2011, 1);
            p->addItem(2020, 1);
            p->addItem(2021, 1);
            p->addItem(2022, 1);
            p->addItem(2023, 1);
        }
        eventPublisher.sendTo(
                dbId, "[CHEAT] Armas de rango agregados al inventario. Equipalo con doble click.");
    } else if (type == CheatType::INFINITE_MANA) {
        // Setea el maná al máximo actual del jugador
        player->toggleInfiniteMana();
        eventPublisher.sendTo(dbId, "[CHEAT] Maná infinito toggled.");
    } else if (type == CheatType::GIVE_GOLD) {
        player->addGold(1000);
        eventPublisher.sendTo(dbId, "[CHEAT] +1.000 de oro agregados.");
    }
}

std::string World::getCreatorPlayerName() const { return this->creatorPlayerName; }
int World::getWorldId() const { return this->worldId; }

bool World::addPlayer(uint32_t dbId, std::string& username, Race race, CharacterClass cls,
                      const std::optional<PlayerPersistData>& savedData) {
    if (entityManager.resolveEntityId(dbId) != 0) {
        return false;
    }

    uint32_t entityId = entityManager.allocateEntityId();

    PlayerConfig baseConfig = characterConfigs.player;
    Race savedRace = race;
    CharacterClass savedClass = cls;

    RaceConfig raceConfig = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig classConfig = {1.0f, 1.0f, 1.0f, false};

    auto defaultRaceIt = characterConfigs.races.find(savedRace);
    auto defaultClassIt = characterConfigs.classes.find(savedClass);
    if (defaultRaceIt != characterConfigs.races.end())
        raceConfig = defaultRaceIt->second;
    if (defaultClassIt != characterConfigs.classes.end())
        classConfig = defaultClassIt->second;

    std::pair<float, float> defaultSpawn = map.getInitialPosition();
    Position spawnPos =
            Position{static_cast<int>(defaultSpawn.first), static_cast<int>(defaultSpawn.second)};

    if (savedData.has_value()) {
        const PlayerPersistData& d = savedData.value();

        Position savedPos{d.posX, d.posY};
        auto freePos = map.findClosestFreePosition(savedPos, 5);
        if (freePos) {
            spawnPos = *freePos;
        } else {
            // Fallback si la zona está asquerosamente llena, buscar desde defaultSpawn
            auto fallbackPos = map.findClosestFreePosition(spawnPos, 10);
            if (fallbackPos)
                spawnPos = *fallbackPos;
        }

        baseConfig.startingLevel = d.level;
        baseConfig.startingExperience = d.exp;

        savedRace = static_cast<Race>(d.race);
        savedClass = static_cast<CharacterClass>(d.characterClass);

        auto raceIt = characterConfigs.races.find(savedRace);
        auto classIt = characterConfigs.classes.find(savedClass);
        if (raceIt != characterConfigs.races.end())
            raceConfig = raceIt->second;
        if (classIt != characterConfigs.classes.end())
            classConfig = classIt->second;
    } else {
        // Jugador nuevo (o sin persistencia) en spawn default
        auto freePos = map.findClosestFreePosition(spawnPos, 10);
        if (freePos) {
            spawnPos = *freePos;
        }
    }

    auto player = std::make_unique<Player>(entityId, dbId, username, savedRace, savedClass,
                                           raceConfig, classConfig, baseConfig, itemRegistry,
                                           inventoryConfig, spawnPos);

    if (savedData.has_value()) {
        player->fromPersistData(savedData.value());
    }

    entityManager.registerPlayer(entityId, dbId, std::move(player));
    map.setEntityCollision(spawnPos.x, spawnPos.y, true);
    return true;
}

bool World::removePlayer(uint32_t dbId) {
    uint32_t entityId = entityManager.resolveEntityId(dbId);
    if (entityId == 0)
        return false;

    interactionService.endInteraction(entityId);

    auto clanIdOpt = clanRepo.getClanIdOfPlayer(dbId);
    if (clanIdOpt) {
        const Clan* clan = clanRepo.getClanById(*clanIdOpt);
        const Player* p = entityManager.getPlayer(dbId);
        if (clan && p) {
            std::string playerName = p->getName();
            for (uint32_t memberId: clan->getMembers()) {
                if (memberId != dbId) {
                    eventPublisher.sendTo(memberId, "[Clan] " + playerName + " salió del juego.");
                }
            }
        }
    }

    const Player* p = entityManager.getPlayer(dbId);
    if (p) {
        map.setEntityCollision(p->getPosition().x, p->getPosition().y, false);
    }

    return entityManager.removePlayer(dbId);
}

bool World::loadMap(const std::string& path, bool spawnMonstersAndItems) {
    Map::MapLoadOptions options;
    options.spawnMonsters = spawnMonstersAndItems;
    options.spawnGroundItems = spawnMonstersAndItems;
    if (map.loadSpawnFromJson(path, options)) {
        spawnNPCs();
        if (spawnMonstersAndItems) {
            for (const auto& req: spawnSystem.getInitialSpawns(map)) {
                addMonster(req.type, req.pos, *req.config);
            }
        }
        return true;
    }
    return false;
}

void World::spawnNPCs() {
    NPCFactory factory(itemRegistry, globalBank);
    for (const auto& spawn: map.getAllNPCs()) {
        uint32_t entityId = entityManager.allocateEntityId();
        if (auto npc = factory.create(entityId, spawn.type, spawn.position)) {
            map.setEntityCollision(npc->getPosition().x, npc->getPosition().y, true);
            entityManager.addNPC(std::move(npc));
        }
    }
}

uint32_t World::addMonster(NPCType type, Position pos, const MonsterConfig& config) {
    uint32_t id = entityManager.addMonster(type, pos, config);
    map.setEntityCollision(pos.x, pos.y, true);
    return id;
}

void World::moveEntity(uint32_t dbId, Movement direction) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    p->onActionStarted();
    Position oldPos = p->getPosition();
    Position candidate = p->tryMove(direction);

    if (!map.canMoveTo(candidate))
        return;

    map.setEntityCollision(oldPos.x, oldPos.y, false);
    p->setPosition(candidate);
    map.setEntityCollision(candidate.x, candidate.y, true);
    p->setAction(static_cast<uint8_t>(EntityAction::WALKING), 200.0f);

    interactionService.endInteraction(entityManager.resolveEntityId(dbId));
}

void World::playerAttack(uint32_t attackerId, uint32_t targetDbId) {
    combatSystem.playerAttack(attackerId, targetDbId);
}

void World::playerShoot(uint32_t shooterDbId, float targetX, float targetY) {
    Player* shooter = entityManager.getPlayer(shooterDbId);
    if (!shooter || shooter->isDead())
        return;

    if (!shooter->canAttack()) {
        eventPublisher.sendTo(shooterDbId, "Aún no puedes disparar.");
        return;
    }
    if (map.isSafeZone(shooter->getPosition().x, shooter->getPosition().y)) {
        eventPublisher.sendTo(shooterDbId, "No puedes disparar en una zona segura.");
        return;
    }

    const Weapon* weapon = shooter->getEquippedWeapon();
    if (!weapon || weapon->getType() == WeaponType::MELEE) {
        eventPublisher.sendTo(shooterDbId, "No tienes un arma de rango equipada.");
        return;
    }

    // --- Flauta élfica: curación instantánea en self ---
    if (weapon->getId() == ITEM_FLAUTA_ELFICA) {
        if (!shooter->consumeMana(weapon->getManaCost())) {
            eventPublisher.sendTo(shooterDbId, "No tienes suficiente maná.");
            return;
        }
        shooter->heal(FLAUTA_HEAL_AMOUNT);
        eventPublisher.sendTo(shooterDbId,
                              "Te curaste " + std::to_string(FLAUTA_HEAL_AMOUNT) + " HP.");
        shooter->onActionStarted();
        return;
    }

    // --- Armas mágicas: consumir maná ---
    if (weapon->getType() == WeaponType::MAGIC) {
        if (!shooter->consumeMana(weapon->getManaCost())) {
            eventPublisher.sendTo(shooterDbId, "No tienes suficiente maná.");
            return;
        }
    }

    // --- Determinar tipo de proyectil y sprite según item ID ---
    ProjectileType pType = ProjectileType::ARROW;
    uint16_t sprite = SPRITE_ARROW;
    float speed = 12.f;
    float range = 15.f;

    switch (weapon->getId()) {
        case ITEM_VARA_FRESNO:
            pType = ProjectileType::MAGIC_ARROW;
            sprite = SPRITE_MAGIC_ARROW;
            speed = 10.f;
            break;
        case ITEM_BACULO_NUDOSO:
            pType = ProjectileType::MISSILE;
            sprite = SPRITE_MISSILE;
            speed = 9.f;
            break;
        case ITEM_BACULO_ENGARZADO:
            pType = ProjectileType::EXPLOSION;
            sprite = SPRITE_EXPLOSION;
            speed = 8.f;
            range = 16.f;
            break;
        case ITEM_ARCO_COMPUESTO:
            speed = 14.f;
            range = 18.f;
            break;
        default:
            break;
    }

    float sx = static_cast<float>(shooter->getPosition().x);
    float sy = static_cast<float>(shooter->getPosition().y);

    projectileSystem.spawnProjectile(
            shooterDbId, sx, sy, targetX, targetY, sprite, weapon->getMinDamage(),
            weapon->getMaxDamage(), weapon->isMagic(), weapon->getHitEffect(), pType, speed, range);

    shooter->onActionStarted();
}

void World::playerInteract(uint32_t dbId, uint32_t targetId) {
    const Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    Interactable* npc = entityManager.findInteractable(targetId);
    InteractionResult res = interactionService.startInteraction(entityManager.resolveEntityId(dbId),
                                                                const_cast<Player&>(*p), npc);
    eventPublisher.sendTo(dbId, res.msg);
}

void World::playerExecuteNpcCommand(uint32_t dbId, const NpcCommandDTO& dto) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    p->onActionStarted();
    uint32_t playerEntityId = entityManager.resolveEntityId(dbId);

    Interactable* targetNpc = resolveNpcTarget(dto.npcId, *p);

    if (!targetNpc) {
        eventPublisher.sendTo(dbId, "[INFO] No hay nadie con quien interactuar allí.");
        return;
    }

    InteractionResult startRes = interactionService.startInteraction(playerEntityId, *p, targetNpc);
    if (startRes.status == InteractionStatus::FAILURE) {
        eventPublisher.sendTo(dbId, "[INFO] " + startRes.msg);
        return;
    }

    InteractionResult res = interactionService.executeCommand(playerEntityId, *p, dto);
    publishInteractionResult(dbId, res);
}

Interactable* World::resolveNpcTarget(uint32_t targetId, const Player& player) const {
    // 1. Búsqueda por coordenadas codificadas en targetId
    if (targetId > 0) {
        int targetX = (targetId >> 16) & 0xFFFF;
        int targetY = targetId & 0xFFFF;

        for (auto& [id, npc]: entityManager.getCityNPCs()) {
            const Position& pos = npc->getPosition();
            if (pos.x == targetX && (pos.y == targetY || pos.y - 1 == targetY))
                return npc.get();
        }

        // Fallback: buscar por id directo en el EntityManager
        if (Interactable* npc = entityManager.findInteractable(targetId))
            return npc;
    }

    // 2. NPC más cercano dentro del rango de interacción
    constexpr int INTERACTION_RANGE = 3;
    Interactable* nearest = nullptr;
    int minDist = INTERACTION_RANGE;

    for (auto& [id, npc]: entityManager.getCityNPCs()) {
        int dist = player.getPosition().chebyshev_distance_to(npc->getPosition());
        if (dist < minDist) {
            minDist = dist;
            nearest = npc.get();
        }
    }
    return nearest;
}

void World::publishInteractionResult(uint32_t dbId, const InteractionResult& res) {
    switch (res.status) {
        case InteractionStatus::SUCCESS:
            eventPublisher.sendTo(dbId, res.msg);
            break;
        case InteractionStatus::FAILURE:
            eventPublisher.sendTo(dbId, "[INFO] " + res.msg);
            break;
        case InteractionStatus::UNHANDLED:
            eventPublisher.sendTo(dbId, "[INFO] El NPC no comprende ese comando.");
            break;
    }
}

Player* World::findNearestPlayer(const Monster& monster, int range) {
    Player* nearest = nullptr;
    int minDist = range + 1;

    for (auto& [id, player]: entityManager.getPlayers()) {
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

void World::moveMonsterTowards(Monster& monster, const Position& targetPos) {
    if (!monster.canMove())
        return;

    Position mPos = monster.getPosition();
    int dx = (targetPos.x > mPos.x) ? 1 : ((targetPos.x < mPos.x) ? -1 : 0);
    int dy = (targetPos.y > mPos.y) ? 1 : ((targetPos.y < mPos.y) ? -1 : 0);

    if (dx == 0 && dy == 0)
        return;

    std::vector<Position> candidates;

    if (dx != 0 && dy != 0) {
        // Movimiento diagonal
        candidates.push_back({mPos.x + dx, mPos.y + dy});
        candidates.push_back({mPos.x + dx, mPos.y});
        candidates.push_back({mPos.x, mPos.y + dy});
    } else if (dx != 0) {
        // Movimiento horizontal
        candidates.push_back({mPos.x + dx, mPos.y});
        candidates.push_back({mPos.x + dx, mPos.y + 1});
        candidates.push_back({mPos.x + dx, mPos.y - 1});
        candidates.push_back({mPos.x, mPos.y + 1});
        candidates.push_back({mPos.x, mPos.y - 1});
    } else if (dy != 0) {
        // Movimiento vertical
        candidates.push_back({mPos.x, mPos.y + dy});
        candidates.push_back({mPos.x + 1, mPos.y + dy});
        candidates.push_back({mPos.x - 1, mPos.y + dy});
        candidates.push_back({mPos.x + 1, mPos.y});
        candidates.push_back({mPos.x - 1, mPos.y});
    }

    auto it = std::find_if(candidates.begin(), candidates.end(),
                           [&](const Position& c) { return c != mPos && map.canMoveTo(c); });

    if (it != candidates.end()) {
        map.setEntityCollision(mPos.x, mPos.y, false);
        monster.setPosition(*it);
        map.setEntityCollision(it->x, it->y, true);
        monster.resetMoveCooldown();
        monster.setAction(static_cast<uint8_t>(EntityAction::WALKING), 200.0f);
    }
}

void World::update(float delta_time) {
    for (auto& [id, player]: entityManager.getPlayers()) {
        player->update(delta_time);
    }

    for (auto& [id, monster]: entityManager.getMonsters()) {
        if (monster->isDead())
            continue;

        monster->update(delta_time);

        Player* target = findNearestPlayer(*monster, monster->get_detection_range());
        if (!target) {
            monster->setTargetId(0);
            continue;
        }

        if (monster->getTargetId() != target->getId()) {
            monster->setTargetId(target->getId());
            monster->resetAttackCooldown();
            monster->resetMoveCooldown();
        }

        int dist = monster->distance_to(*target);

        if (dist <= monster->get_attack_range()) {
            if (monster->canAttack()) {
                combatSystem.monsterAttack(*monster, *target);
                monster->resetAttackCooldown();
                monster->setAction(static_cast<uint8_t>(EntityAction::ATTACKING), 400.0f);
            }
        } else {
            moveMonsterTowards(*monster, target->getPosition());
        }
    }

    auto completedResurrections = resurrectionService.tick(delta_time);
    for (const auto& res: completedResurrections) {
        Player* player = entityManager.getPlayer(res.playerDbId);
        if (player) {
            map.setEntityCollision(player->getPosition().x, player->getPosition().y, false);

            Position finalPos = res.targetPos;
            auto freePos = map.findClosestFreePosition(res.targetPos, 5);
            if (freePos)
                finalPos = *freePos;

            player->setPosition(finalPos);
            map.setEntityCollision(finalPos.x, finalPos.y, true);
            player->resurrect();
            eventPublisher.sendTo(res.playerDbId, "¡Has sido resucitado!");
        }
    }

    for (uint32_t deadId: deadMonsterIds) {
        auto it = entityManager.getMonsters().find(deadId);
        if (it != entityManager.getMonsters().end()) {
            map.setEntityCollision(it->second->getPosition().x, it->second->getPosition().y, false);
        }
        entityManager.eraseMonster(deadId);
    }
    deadMonsterIds.clear();

    auto newSpawns =
            spawnSystem.tick(delta_time, entityManager.getMonsterCount(), map, entityManager);
    for (const auto& req: newSpawns) {
        entityManager.addMonster(req.type, req.pos, *req.config);
        map.setEntityCollision(req.pos.x, req.pos.y, true);
    }

    projectileSystem.update(delta_time);
}

std::vector<WorldEvent> World::pollEvents() { return eventPublisher.pollEvents(); }

SnapshotDTO World::generateSnapshot() const {
    SnapshotDTO snapshot;
    for (const auto& pair: entityManager.getMonsters()) {
        snapshot.monsters.push_back(pair.second->toEntityDTO());
    }

    for (const auto& pair: entityManager.getPlayers()) {
        snapshot.players.push_back(pair.second->toEntityDTO());
    }

    for (const auto& pair: map.getGroundItemsSnapshot()) {
        const Position& pos = pair.first;
        const GroundItem& item = pair.second;
        snapshot.groundItems.push_back(GroundItemDTO(item.itemId, item.amount, pos.x, pos.y));
    }

    snapshot.projectiles = projectileSystem.getProjectileDTOs();

    return snapshot;
}

std::optional<PlayerStatsDTO> World::getPlayerStatsDTO(uint32_t dbId) const {
    const Player* player = entityManager.getPlayer(dbId);
    if (!player)
        return std::nullopt;

    return player->getStatsDTO();
}
int World::getPlayerCount() const { return static_cast<int>(entityManager.getPlayerCount()); }
bool World::isEmpty() const { return entityManager.isEmpty(); }

std::optional<Position> World::getPlayerPosition(uint32_t dbId) const {
    return entityManager.getPlayerPosition(dbId);
}

std::optional<std::string> World::getPlayerUsername(uint32_t dbId) const {
    return entityManager.getPlayerUsername(dbId);
}

std::vector<uint32_t> World::getOnlinePlayerDbIds() const {
    return entityManager.getOnlinePlayerDbIds();
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

void World::pickUpItem(uint32_t dbId) {
    auto posOpt = getPlayerPosition(dbId);
    if (!posOpt)
        return;

    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (p->isDead()) {
        eventPublisher.sendTo(dbId, "No puedes hacer eso siendo un fantasma.");
        return;
    }

    auto itemOpt = map.pickUpItem(posOpt.value());
    if (!itemOpt) {
        eventPublisher.sendTo(dbId, "No hay objetos aquí para recoger.");
        return;
    }

    p->setAction(static_cast<uint8_t>(EntityAction::GRABBING), 500.0f);

    if (itemOpt->itemId == GOLD_ITEM_ID) {
        p->addGold(itemOpt->amount);
        eventPublisher.sendTo(dbId,
                              "Recogiste " + std::to_string(itemOpt->amount) + " monedas de oro.");
    } else {
        uint16_t leftover = p->addInventoryItem(itemOpt->itemId, itemOpt->amount);

        if (leftover > 0) {
            eventPublisher.sendTo(dbId, "Inventario lleno. No pudiste recoger todo.");
            map.placeItem(posOpt.value(), itemOpt->itemId, leftover);
        } else {
            eventPublisher.sendTo(dbId, "Objeto recogido.");
        }
    }
}

void World::playerMeditate(uint32_t dbId) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (p->isDead()) {
        eventPublisher.sendTo(dbId, "No puedes hacer eso siendo un fantasma.");
        return;
    }

    p->startMeditating();
}

void World::dropItem(uint32_t dbId, uint8_t slot, uint16_t amount) {
    auto posOpt = getPlayerPosition(dbId);
    if (!posOpt)
        return;

    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (p->isDead()) {
        eventPublisher.sendTo(dbId, "No puedes hacer eso siendo un fantasma.");
        return;
    }

    auto slotOpt = p->inspectInventorySlot(slot);
    if (!slotOpt || slotOpt->amount == 0)
        return;

    // amount == 0 significa "tirar todo el stack"; si amount > disponible, clampear
    uint16_t effectiveAmount = amount;
    if (effectiveAmount == 0 || effectiveAmount > slotOpt->amount) {
        effectiveAmount = slotOpt->amount;
    }

    auto placedPos = map.placeItemNearby(posOpt.value(), slotOpt->item_id, effectiveAmount);
    if (!placedPos) {
        eventPublisher.sendTo(dbId, "No hay suficiente espacio en el suelo para tirar el objeto.");
        return;
    }

    p->removeInventoryItem(slot, effectiveAmount);
}

void World::equipItem(uint32_t dbId, uint8_t slot) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (p->isDead()) {
        eventPublisher.sendTo(dbId, "No puedes hacer eso siendo un fantasma.");
        return;
    }

    if (p->equipFromSlot(slot)) {
        eventPublisher.sendTo(dbId, "Equipaste el item.");
    }
}

void World::useItem(uint32_t dbId, uint8_t slot) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (p->isDead()) {
        eventPublisher.sendTo(dbId, "No puedes hacer eso siendo un fantasma.");
        return;
    }

    if (!p->canUseItems()) {
        eventPublisher.sendTo(dbId, "No puedes usar items en este momento.");
        return;
    }

    auto slotOpt = p->inspectInventorySlot(slot);
    if (!slotOpt || slotOpt->amount == 0)
        return;

    const Consumable* consumable = itemRegistry.get_consumable(slotOpt->item_id);
    if (!consumable) {
        eventPublisher.sendTo(dbId, "Ese objeto no es consumible.");
        return;
    }

    if (consumable->use(*p)) {
        p->removeInventoryItem(slot, 1);
    } else {
        eventPublisher.sendTo(dbId, "No necesitas usar eso en este momento.");
    }
}

void World::handlePlayerDeath(uint32_t dbId) {
    Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    Position pos = p->getPosition();
    map.setEntityCollision(pos.x, pos.y, false);

    p->onActionStarted();

    uint32_t dropped_gold = p->dropExcessGold();
    if (dropped_gold > 0) {
        while (dropped_gold > 0) {
            uint16_t chunk = static_cast<uint16_t>(std::min(dropped_gold, uint32_t(UINT16_MAX)));
            map.placeItemNearby(pos, GOLD_ITEM_ID, chunk);
            dropped_gold -= chunk;
        }
    }

    std::vector<Slot> dropped_items = p->dropAllItems();
    for (const auto& slot: dropped_items) {
        map.placeItemNearby(pos, slot.item_id, slot.amount);
    }
}

void World::playerResurrect(uint32_t dbId) {
    const Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return;

    if (!p->isDead()) {
        eventPublisher.sendTo(dbId, "No eres un fantasma.");
        return;
    }

    std::vector<Position> priestPositions;
    for (auto& [id, npc]: entityManager.getCityNPCs()) {
        if (dynamic_cast<Priest*>(npc.get())) {
            priestPositions.push_back(npc->getPosition());
        }
    }

    auto res = resurrectionService.requestResurrection(dbId, p->getPosition(), p->isDead(),
                                                       priestPositions);
    eventPublisher.sendTo(dbId, res.message);
}

void World::onMonsterDeath(const Monster& monster, uint32_t killerDbId) {
    Position pos = monster.getPosition();

    auto potionIds = itemRegistry.getPotionIds();
    auto allItemIds = itemRegistry.getAllDroppableItemIds();

    NpcLootResult loot = LootResolver::resolveNpcLoot(monster.getMaxHp(), potionIds, allItemIds);

    if (loot.dropsGold && loot.goldAmount > 0) {
        uint32_t gold = loot.goldAmount;
        while (gold > 0) {
            uint16_t chunk =
                    static_cast<uint16_t>(std::min(gold, static_cast<uint32_t>(UINT16_MAX)));
            map.placeItemNearby(pos, GOLD_ITEM_ID, chunk);
            gold -= chunk;
        }
        eventPublisher.sendTo(killerDbId, "La criatura dejo " + std::to_string(loot.goldAmount) +
                                                  " monedas de oro.");
    }

    if (loot.dropsItem && loot.droppedItemId > 0) {
        map.placeItemNearby(pos, loot.droppedItemId, 1);
        const Item* item = itemRegistry.get_item(loot.droppedItemId);
        std::string itemName = item ? item->getName() : "objeto desconocido";
        eventPublisher.sendTo(killerDbId, "La criatura dejo: " + itemName + ".");
    }

    deadMonsterIds.push_back(monster.getId());
}

void World::onPlayerDeath(uint32_t dbId) { handlePlayerDeath(dbId); }

uint16_t World::getPlayerLevel(uint32_t dbId) const { return entityManager.getPlayerLevel(dbId); }

uint32_t World::resolveNickToDbId(const std::string& nick) const {
    return entityManager.resolveNickToDbId(nick);
}

void World::processClanCommand(uint32_t senderDbId, const ClanCommandDTO& cmd) {
    std::vector<ClanNotification> notifs;
    clanController.dispatch(senderDbId, cmd, *this, notifs);
    for (const auto& n: notifs) {
        eventPublisher.sendTo(n.targetDbId, n.message);
    }
}

bool World::areClanmates(uint32_t a, uint32_t b) const { return combatSystem.areClanmates(a, b); }

Player* World::getPlayerById(uint32_t dbId) { return entityManager.getPlayer(dbId); }

ClanRepositoryPersistData World::getClansPersistData() const { return clanRepo.toPersistData(); }

void World::restoreClans(const ClanRepositoryPersistData& data) { clanRepo.fromPersistData(data); }

BankPersistData World::getBankPersistData() const { return globalBank.toPersistData(); }

void World::restoreBank(const BankPersistData& data) { globalBank.fromPersistData(data); }


std::vector<MonsterPersistData> World::getMonstersPersistData() const {
    std::vector<MonsterPersistData> data;
    auto& monstersMap = const_cast<EntityManager&>(entityManager).getMonsters();
    data.reserve(monstersMap.size());
    std::transform(monstersMap.begin(), monstersMap.end(), std::back_inserter(data),
                   [](const auto& pair) { return pair.second->toPersistData(); });
    return data;
}

void World::restoreMonsters(const std::vector<MonsterPersistData>& data,
                            const MonsterConfigs& configs) {
    for (const auto& md: data) {
        auto type = static_cast<NPCType>(md.type);
        auto it = configs.find(type);
        if (it == configs.end()) {
            continue;
        }

        uint32_t entityId = md.entityId;
        Position pos{md.posX, md.posY};

        auto monster = std::make_unique<Monster>(entityId, type, pos, it->second);
        monster->fromPersistData(md);

        entityManager.addMonster(std::move(monster));
        map.setEntityCollision(pos.x, pos.y, true);
    }
}

std::pair<std::vector<NpcHeaderPersistData>, std::vector<std::vector<NpcStockPersistData>>>
        World::getNpcsPersistData() const {
    std::vector<NpcHeaderPersistData> headers;
    std::vector<std::vector<NpcStockPersistData>> allStocks;

    // Recorremos los NPCs de la ciudad indexados por el EntityManager
    for (const auto& [id, npc]: entityManager.getCityNPCs()) {
        // Intentamos castear a las clases que manejan stock dinámico
        std::unordered_map<uint32_t, int> currentStock;
        uint8_t npcType = 0;

        if (const auto* merchant = dynamic_cast<const Merchant*>(npc.get())) {
            currentStock = merchant->getStock();
            npcType = 1;  // ID de tipo para Merchant
        } else if (const auto* priest = dynamic_cast<const Priest*>(npc.get())) {
            currentStock = priest->getStock();
            npcType = 2;  // ID de tipo para Priest
        } else {
            continue;  // Si es un Banker u otro interactuable sin stock transaccional, salteamos
        }

        NpcHeaderPersistData header{};
        header.entityId = npc->getId();
        header.type = npcType;
        header.posX = npc->getPosition().x;
        header.posY = npc->getPosition().y;
        header.stockCount = static_cast<uint32_t>(currentStock.size());

        std::vector<NpcStockPersistData> stockData;
        for (const auto& [itemId, amount]: currentStock) {
            stockData.push_back(NpcStockPersistData{itemId, amount});
        }

        headers.push_back(header);
        allStocks.push_back(stockData);
    }

    return {headers, allStocks};
}

void World::restoreNpcStates(const std::vector<NpcHeaderPersistData>& headers,
                             const std::vector<std::vector<NpcStockPersistData>>& allStocks) {

    for (size_t i = 0; i < headers.size(); ++i) {
        const auto& header = headers[i];
        const auto& stockData = allStocks[i];

        // Reconstruimos el mapa de stock binario a unordered_map
        std::unordered_map<uint32_t, int> restoredStock;
        for (const auto& item: stockData) {
            restoredStock[item.itemId] = item.amount;
        }

        // Buscamos cuál de los NPCs recién spawneados por el mapa coincide en posición
        for (auto& [id, npc]: entityManager.getCityNPCs()) {
            if (npc->getPosition().x == header.posX && npc->getPosition().y == header.posY) {
                if (header.type == 1) {
                    if (auto* merchant = dynamic_cast<Merchant*>(npc.get())) {
                        merchant->setStock(restoredStock);
                    }
                } else if (header.type == 2) {
                    if (auto* priest = dynamic_cast<Priest*>(npc.get())) {
                        priest->setStock(restoredStock);
                    }
                }
                break;
            }
        }
    }
}

std::vector<GroundItemPersistData> World::getGroundItemsPersistData() const {
    return map.getGroundItemsPersistData();
}

void World::restoreGroundItems(const std::vector<GroundItemPersistData>& data) {
    map.restoreGroundItems(data);
}

std::optional<PlayerPersistData> World::getPlayerPersistData(uint32_t dbId) const {
    const Player* p = entityManager.getPlayer(dbId);
    if (!p)
        return std::nullopt;
    return p->toPersistData();
}
