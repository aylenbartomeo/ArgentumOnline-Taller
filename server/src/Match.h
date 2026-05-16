#ifndef MATCH_H
#define MATCH_H

#include "GameLoop.h"
#include "../common/include/queue.h"
#include "../common/src/CommandDTO.h"
#include "../common/src/Snapshot.h"

/**
 * @class Match
 * @brief Esta clase representaría una partida en el juego
 * Responsabilidad: agregar/eliminar jugadoress, gestionar el estado de la partida.
 */

class Match {
private:
    int matchId;
    std::string creatorPlayerName;
    Queue<CommandDTO> queueCMD;
    std::atomic<bool> isOnline;
    int cantPlayers;
    int cantMaxPlayers;
    GameLoop gameLoop;

public:
    explicit Match(int matchId, const std::string& creatorPlayerName);
    
    std::string getCreatorPlayerName() const;
    int getMatchId() const;
    bool addPlayer(int playerId, const std::string& playerName, Queue<Snapshot>& sender_queue);
    bool isFull() const;  
    bool removePlayer(int playerId);
    Queue<CommandDTO>& getQueue();
    std::string getMap();

    ~Match();

};

#endif
