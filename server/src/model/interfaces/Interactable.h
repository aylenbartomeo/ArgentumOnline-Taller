#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include <vector>
#include "position.h"
#include "dto/ClientCommands.h"

class Player;
struct WorldEvent; // Forward declaration de tus eventos salientes

class Interactable {
public:
    virtual ~Interactable() = default;
    
    // Retorna la posición física del NPC para que el World valide la distancia
    virtual Position getPosition() const = 0;

    // El NPC recibe el click inicial del jugador y genera los eventos de red necesarios
    // (Por ejemplo: indicarle al cliente que abra la interfaz de comercio o banco)
    virtual void beInteractedBy(Player& player, std::vector<WorldEvent>& outgoingEvents) = 0;

    // El NPC procesa el comando específico (BUY, SELL, DEPOSIT, etc.) enviado por el jugador
    virtual void handleCommand(Player& player, const NpcCommandDTO& dto, std::vector<WorldEvent>& outgoingEvents) = 0;
};

#endif
