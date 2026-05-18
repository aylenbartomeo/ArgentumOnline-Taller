#include "World.h"
#include <iostream>
#include "model/entities/Player.h"

World::World(int worldId, const std::string& creatorPlayerName):
        worldId(worldId),
        creatorPlayerName(creatorPlayerName),
        map() {}

std::string World::getCreatorPlayerName() const {
    return this->creatorPlayerName;
}

int World::getWorldId() const {
    return this->worldId;
}

bool World::addPlayer(uint32_t playerId, std::string& username) {
    if (this->players.find(playerId) != this->players.end()) {
        return false;
    }
    CombatManager* dummy_combat = nullptr;
    ItemRegistry* dummy_registry = nullptr;

    // Llenamos los 11 argumentos que exige Player.h
    this->players[playerId] = std::make_unique<Player>(
        playerId, 
        username, 
        Race::HUMAN,              // 3. Race
        CharacterClass::WARRIOR,  // 4. CharacterClass
        Position{0,0},            // 5. Position
        *dummy_combat,            // 6. CombatManager&
        PlayerConfig{},           // 7. PlayerConfig
        RaceConfig{},             // 8. RaceConfig
        CharacterClassConfig{},   // 9. CharacterClassConfig
        InventoryConfig{},        // 10. InventoryConfig
        *dummy_registry           // 11. ItemRegistry&
    );

    return true;
}

bool World::removePlayer(uint32_t playerId) {
    auto it = this->players.find(playerId);
    if (it == this->players.end()) {
        return false; // El jugador no pertenecía a este mundo
    }

    this->players.erase(it);
    return true;
}

void World::moveEntity(uint32_t playerId, Movement movement) {
    // 1. Buscás al jugador en tu mapa de players
    // 2. Le pedís su posición actual
    // 3. Le preguntás a Map: ¿Es válida la posición de destino (actual + dirección)?
    // 4. Si no hay colisión ni límite de mapa, actualizás la posición del jugador.
    (void)playerId;
    (void)movement;
}

void World::playerAttack(uint32_t playerId) {
    (void)playerId;
}

Queue<GameEvent>& World::getQueue() {
    return this->queueCMD;
}

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
    
    // Mapeamos el estado actual de los jugadores del modelo al DTO de red
    // Suponiendo que tu SnapshotDTO tiene un diccionario o vector de posiciones:
    for (const auto& pair : this->players) {
        uint32_t id = pair.first;
        // const Player& player = pair.second;
        
        // Simulación de llenado de DTO según tus estructuras comunes:
        // PlayerDataDTO data;
        // data.x = player.getX();
        // data.y = player.getY();
        // snapshot.playersData[id] = data;
        (void)id; 
    }

    return snapshot;
}

int World::getPlayerCount() const {
    return static_cast<int>(this->players.size());
}

bool World::isEmpty() const {
    return this->players.empty();
}
