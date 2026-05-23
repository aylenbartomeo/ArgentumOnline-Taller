#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/model/ServerEvents.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "model/entities/Player.h"

#include "Map.h"
#include "queue.h"

class World {
private:
    int worldId;
    std::string creatorPlayerName;

    Map map;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;

public:
    explicit World(int worldId, const std::string& creatorPlayerName);

    // Métodos lógicos: Entrar y salir del mundo virtual
    bool addPlayer(uint32_t playerId, std::string& username);
    bool removePlayer(uint32_t playerId);

    /* Metodos de acciones de los personajes en el mundo */
    void moveEntity(uint32_t playerId, Movement movement);

    // void playerAttack(AttackDTO& dto);

    /* actualización del estado del mundo */
    void update(float delta_time);

    // Generación del estado actual para ser enviado por red
    SnapshotDTO generateSnapshot() const;

    /* Getters y setters */
    std::string getCreatorPlayerName() const;
    int getWorldId() const;
    int getPlayerCount() const;
    bool isEmpty() const;  // Para que el servidor sepa cuándo destruir la partida

    ~World() = default;
};

#endif
