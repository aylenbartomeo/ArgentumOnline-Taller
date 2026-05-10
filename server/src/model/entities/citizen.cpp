#include "citizen.h"

Citizen::Citizen(uint32_t id, Position pos, NPCType type)
{
}

void Citizen::interact(Interactable &interactable, const std::string &action, const std::vector<std::string> &params)
{
    // Aca deberia ocurrir la interaccion con el jugador, dependiendo del tipo de NPC y la accion, 
    // se llamaria a los metodos correspondientes (buy, sell, heal, respawn, etc).
    // Habria que generar la comunicacion dado que ambos son Interactables
}
