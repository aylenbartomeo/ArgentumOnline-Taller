#ifndef INTERACTABLE_H
#define INTERACTABLE_H

class Player; // Forward declaration

class Interactable {
public:
    virtual ~Interactable() = default;
    
    // El NPC recibe al jugador y decide qué pantalla abrirle o qué hacer con él
    virtual void beInteractedBy(Player& player) = 0;
};

#endif
