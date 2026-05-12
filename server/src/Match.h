#ifndef MATCH_H
#define MATCH_H

#include "model/entities/player.h"
#include "Map.h"

/**
 * @class Match
 * @brief Esta clase representaría una partida en el juego
 * Responsabilidad: Orquestar la interacción entre jugadores, monstruos e ítems dentro de un mapa específico.
 */
class Match {
private:
    int matchId;
    Map map;
    std::map<int, Player&> players;
public:
    explicit Match();
};

#endif
