#ifndef NPC_COMMAND_H_
#define NPC_COMMAND_H_

#include <string>
#include <vector>

class Player;

class NPCCommand {
public:
    virtual ~NPCCommand() = default;

    // Ejecuta la acción transaccional sobre el jugador.
    // player El jugador que solicitó la interacción.
    // params Parámetros en crudo recibidos desde la red.
    // Retorna true si la transacción fue exitosa, false si falló (ej. sin oro/espacio).
    virtual bool execute(Player& player, const std::vector<std::string>& params) = 0;
};

#endif  // NPC_COMMAND_H_
