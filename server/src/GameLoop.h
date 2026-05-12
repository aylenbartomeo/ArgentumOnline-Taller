#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "../common/include/thread.h"

/**
 * @class GameLoop
 * @brief Esta clase representaría el bucle principal del juego
 * Responsabilidad: Gestionar el ciclo de actualización del juego, incluyendo la lógica de juego, la interacción entre entidades y la sincronización con los clientes.
 */
class GameLoop : public Thread {
private:

public:
    explicit GameLoop();

    void add_player_to_match(int playerId, int matchId);
    void remove_player_from_match(int playerId, int matchId); 

    virtual void run() override;

     ~GameLoop() override;
};

#endif
