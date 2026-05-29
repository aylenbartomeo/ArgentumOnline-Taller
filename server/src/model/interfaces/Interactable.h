#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include <vector>

#include "dto/ClientCommands.h"

#include "position.h"

class Player;

class Interactable {
public:
    virtual ~Interactable() = default;

    // Retorna la posición física del NPC para que el World valide la distancia
    virtual Position getPosition() const = 0;

    // El NPC recibe el click inicial del jugador y genera los eventos de red necesarios
    // (Por ejemplo: indicarle al cliente que abra la interfaz de comercio o banco)
    // Nota: el envío de `WorldEvent` hacia la UI cliente está pendiente de acople.
    virtual void beInteractedBy(Player& player) = 0;

    // El NPC procesa el comando específico (BUY, SELL, DEPOSIT, etc.) enviado por el jugador
    // Nota: el envío de `WorldEvent` hacia la UI cliente está pendiente de acople.
    virtual void handleCommand(Player& player, const NpcCommandDTO& dto) = 0;
};

#endif
