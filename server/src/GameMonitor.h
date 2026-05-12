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
 * @brief Pendiente de implementación. Este monitor se encargaría de gestionar el estado global del juego,
 * como el listado de partidas activas, la asignación de jugadores a partidas, etc.
 */
class GameMonitor {
private:
    std::mutex mtx;
    std::map<int, std::unique_ptr<Match>> Matches;

public:
    explicit GameMonitor();

    int create_match(int playerId);
    bool join_match(int playerId, int matchId);
    bool leave_match(int playerId, int matchId);
    std::vector<std::string> list_active_matches();
    void delete_all_matches();

    /* Deshabilito las copias */
    GameMonitor(const GameMonitor&) = delete;
    GameMonitor& operator=(const GameMonitor&) = delete;

    /* Permito el movimiento */
    GameMonitor(GameMonitor&&) = default;
    GameMonitor& operator=(GameMonitor&&) = default;

    ~GameMonitor();

};

#endif
