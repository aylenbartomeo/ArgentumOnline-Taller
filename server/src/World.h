#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "queue.h"
#include "dto/CommandDTO.h"
#include "dto/Snapshot.h"
#include "Map.h"            
#include "model/entities/PlayerMock.h"
#include "../include/model/ServerEvents.h"

class World {
private:
    int worldId;
    std::string creatorPlayerName;
    
    // La cola de entrada única de la partida donde los Receivers de los clientes 
    // depositan comandos concurrentemente.
    Queue<GameEvent> queueCMD; 
    
    Map map;
    std::unordered_map<uint32_t, std::unique_ptr<PlayerMock>> players;

public:
    explicit World(int worldId, const std::string& creatorPlayerName);

    // Métodos lógicos: Entrar y salir del mundo virtual
    bool addPlayer(uint32_t playerId, std::string& username);
    bool removePlayer(uint32_t playerId);

    /* Metodos de acciones de los personajes en el mundo */
    void moveEntity(uint32_t playerId, Movement movement);
    void playerAttack(uint32_t playerId);
    
    /* actualización del estado del mundo */
    void update(float delta_time);
    
    // Generación del estado actual para ser enviado por red
    SnapshotDTO generateSnapshot() const;

    /* Getters y setters */
    Queue<GameEvent>& getQueue();
    std::string getCreatorPlayerName() const;
    int getWorldId() const;
    int getPlayerCount() const; 
    bool isEmpty() const; // Para que el servidor sepa cuándo destruir la partida

    ~World() = default;
};

#endif
