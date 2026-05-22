#ifndef PLAYER_MOCK_H
#define PLAYER_MOCK_H

#include <cstdint>
#include <string>

#include "position.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include "../components/EquipmentComponent.h"
#include "../components/BankComponent.h"
#include "../components/StateComponent.h"
#include "../components/CombatComponent.h"
#include "../interfaces/Combatant.h"
#include "combat/CombatManager.h"

/*
 * Esta clase es un mock súper básico para representar a un jugador en el mundo.
 * No tiene lógica de juego ni interacciones, solo atributos esenciales y métodos de acceso.
 * La idea es que sea fácil de usar en tu Snapshot para mostrar información del jugador
 * sin complicaciones de configuración o lógica de juego.
 */

class Player : Combatant {
private:
    uint32_t id;
    std::string name;
    Position pos;
    StatsComponent stats; 
    InventoryComponent inventory;
    EquipmentComponent equipment;
    BankComponent bank;
    StateComponent state;
    CombatComponent combat;

public:
    Player(uint32_t id, 
                const std::string& name, 
                const RaceConfig& race, 
                const CharacterClassConfig& characterClass,
                const PlayerConfig& playerBase);

    // Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
    // de valores random
    Player(uint32_t id, const std::string& name, const RaceConfig& race, 
               const CharacterClassConfig& characterClass,
               const PlayerConfig& playerBase,
               const FormulaEngine& testEngine);
    
    // -- GETTERS/SETTERS --
    uint32_t getId() const { return this->id; }
    std::string getName() const { return this->name; }
    Position getPosition() const override { return this->pos; }
    void setPosition(const Position& newPos) { this->pos = newPos; }
    
    // -- LOGICA DE NEGOCIO
    void gainExperience(uint32_t amount) { stats.addExperience(amount); }

    // IMPLEMENTACIÓN DE LA INTERFAZ COMBATANT
    void receiveDamage(int amount) override { this->combat.takeDamage(amount); }
    bool isDead() const override { return this->stats.getHp() <= 0; }
    uint16_t getStrength() const override { return this->stats.getStrength(); }
    uint16_t getAgility() const override { return this->stats.getAgility(); }
    uint16_t getTotalDefense() const override { return this->equipment.getDefense(); }
    
    void attack(Combatant& target) override {
        if (this->isDead()) return;
        Weapon* myWeapon = this->equipment.getEquippedWeapon();
        CombatManager::getInstance().executeAttack(*this, target, myWeapon);
    }

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

