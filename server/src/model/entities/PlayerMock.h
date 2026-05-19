#ifndef PLAYER_MOCK_H
#define PLAYER_MOCK_H

#include <cstdint>
#include <string>

#include "position.h"

/*
 * Esta clase es un mock súper básico para representar a un jugador en el mundo.
 * No tiene lógica de juego ni interacciones, solo atributos esenciales y métodos de acceso.
 * La idea es que sea fácil de usar en tu Snapshot para mostrar información del jugador
 * sin complicaciones de configuración o lógica de juego.
 */
class PlayerMock {
private:
    uint32_t id;
    std::string name;
    Position pos;
    uint16_t health;
    uint16_t max_health;

public:
    // Constructor ultra liviano y limpio de configs
    PlayerMock(uint32_t id, const std::string& name):
            id(id), name(name), pos({0, 0}), health(100), max_health(100) {}

    ~PlayerMock() = default;

    // Getters básicos que va a consumir tu Snapshot
    uint32_t getId() const { return this->id; }
    std::string getName() const { return this->name; }

    uint16_t getHp() const { return this->health; }
    uint16_t getMaxHp() const { return this->max_health; }

    // Métodos de posición para el movimiento del World
    Position getPosition() const { return this->pos; }

    void setPosition(const Position& newPos) { this->pos = newPos; }
};

#endif
