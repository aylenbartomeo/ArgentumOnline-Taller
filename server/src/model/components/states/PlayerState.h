#ifndef PLAYER_STATE_H
#define PLAYER_STATE_H

class PlayerState {
public:
    virtual ~PlayerState() = default;

    // Filtros de acciones
    virtual bool canMove() const = 0;
    virtual bool canAttack() const = 0;
    virtual bool canUseItems() const = 0;

    // Filtros de interaccion
    virtual bool canBeAttacked() const = 0;

    // identificadores de estado
    virtual bool isGhost() const { return false; }
    virtual bool isMeditating() const { return false; }
};

#endif
