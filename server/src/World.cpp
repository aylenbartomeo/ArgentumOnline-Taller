#include "World.h"

#include <iostream>

#include "model/entities/Player.h"

World::World(int worldId, const std::string& creatorPlayerName):
        worldId(worldId), creatorPlayerName(creatorPlayerName), map() {
    map.setDimensions(20, 15);
}

std::string World::getCreatorPlayerName() const { return this->creatorPlayerName; }

int World::getWorldId() const { return this->worldId; }

bool World::addPlayer(uint32_t playerId, std::string& username) {
    if (this->players.find(playerId) != this->players.end()) {
        return false;
    }

    PlayerConfig baseConfig = {15, 15, 15, 15, 1, 0, 0};
    RaceConfig raceConfig = {1.0f, 1.0f, 1.0f};
    CharacterClassConfig classConfig = {1.0f, 1.0f, 1.0f, false};

    this->players[playerId] = std::make_unique<Player>(playerId, username, raceConfig, classConfig, baseConfig);

    return true;
}
bool World::removePlayer(uint32_t playerId) {
    auto it = this->players.find(playerId);
    if (it == this->players.end()) {
        return false;  // El jugador no pertenecía a este mundo
    }

    this->players.erase(it);
    return true;
}

void World::moveEntity(uint32_t playerId, Movement direction) {
    auto it = this->players.find(playerId);
    if (it == this->players.end())
        return;

    Player& player = *(it->second);
    Position pos = player.getPosition();

    switch (direction) {
        case Movement::UP:
            pos.y -= 1;
            break;
        case Movement::DOWN:
            pos.y += 1;
            break;
        case Movement::LEFT:
            pos.x -= 1;
            break;
        case Movement::RIGHT:
            pos.x += 1;
            break;
        default:
            break;  // Ignoramos diagonales o STOP por ahora
    }

    if (pos.x < 0 || pos.x >= map.widthLimit() || pos.y < 0 || pos.y >= map.heightLimit()) {
        return;
    }

    player.setPosition(pos);
}

void World::playerAttack(uint32_t playerId) { (void)playerId; }

void World::update(float delta_time) {
    // Evitamos advertencias de compilación si delta_time no se usa en stubs
    (void)delta_time;

    // En un futuro, se recorreran las entidades para actualizar su lógica (ej. veneno, maná, etc.)
    // for (auto& pair : this->players) {
    //     pair.second.update(delta_time);
    // }
}

SnapshotDTO World::generateSnapshot() const {
    SnapshotDTO snapshot;
    uint16_t spriteId = 1;
    for (const auto& pair: this->players) {
        uint32_t id = pair.first;
        const Player& player = *(pair.second);
        Position pos = player.getPosition();

        // Creamos el DTO de la entidad con datos mockeados/reales para el MVP
        EntityDTO entityData;
        entityData.id = id;
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
