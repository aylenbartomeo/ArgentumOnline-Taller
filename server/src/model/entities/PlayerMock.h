#ifndef PLAYER_MOCK_H
#define PLAYER_MOCK_H

#include <cstdint>
#include <string>

#include "position.h"
#include "components/StatsComponent.h"
#include "components/InventoryComponent.h"
#include "components/EquipmentComponent.h"
#include "components/BankComponent.h"
#include "components/StateComponent.h"
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
    StatsComponent stats; 
    InventoryComponent inventory;
    EquipmentComponent equipment;
    BankComponent bank;
    StateComponent state;

public:
    PlayerMock(uint32_t id, const std::string& name) :
        id(id), 
        name(name), 
        pos({0, 0}), 
        // Stats ahora solo maneja combate (sin max_gold)
        stats(15, 12, 14, 16, 100, 40),
        // Inventario ahora absorbe la economía: 20 slots, 5000 seguro, 100000 tope máximo
        inventory(20, 5000, 100000),
        equipment(),
        bank(50, 999999),
        state() {}

    uint32_t getId() const { return this->id; }
    std::string getName() const { return this->name; }
    Position getPosition() const { return this->pos; }
    void setPosition(const Position& newPos) { this->pos = newPos; }

    // Acceso a los componentes
    StatsComponent& getStats() { return this->stats; }
    const StatsComponent& getStats() const { return this->stats;}

    InventoryComponent& getInventory() { return this->inventory; }
    const InventoryComponent& getInventory() const { return this->inventory; }

    EquipmentComponent& getEquipment() { return this->equipment; }
    const EquipmentComponent& getEquipment() const { return this->equipment; }

    BankComponent& getBank() { return this->bank; }

    StateComponent& getState() { return this->state; }
    const StateComponent& getState() const { return this->state; }
};

#endif

