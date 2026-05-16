#ifndef GAMEMONITOR_H
#define GAMEMONITOR_H

#include <list>
#include <map>
#include <mutex>

#include "../common/include/queue.h"
#include "Map.h"
#include "Match.h"

/**
 * @class GameMonitor
 * @brief Pendiente de implementación. 
 */
class GameMonitor {
private:
    std::mutex mtx;
    std::map<int, std::unique_ptr<Match>> matches;
    int GameId = 0;

public:
    GameMonitor();
    int createMatch(int playerId, std::string& creatorPlayerName, Queue<Snapshot>& senderQueue);
    bool joinMatch(int playerId, int matchId, std::string& playerName, Queue<Snapshot>& senderQueue);
    void removePlayerFromMatch(int playerId, int matchId);
    std::vector<std::string> listActiveMatches();
    void deleteAllMatches();

    std::string getMapFromId(int matchId);
    Queue<CommandDTO>* getQueueFromMatch(int matchId);

    /* Deshabilito las copias */
    GameMonitor(const GameMonitor&) = delete;
    GameMonitor& operator=(const GameMonitor&) = delete;

    /* Permito el movimiento */
    GameMonitor(GameMonitor&&) = default;
    GameMonitor& operator=(GameMonitor&&) = default;

    ~GameMonitor() = default;
};

#endif
