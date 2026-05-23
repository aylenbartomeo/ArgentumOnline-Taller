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
#include "../components/RegenerationComponent.h"
#include "../interfaces/Attackable.h"
#include "../combat/CombatManager.h"

class Player : Attackable {
private:
    uint32_t id;
    std::string name;
    Position pos;
    StatsComponent stats; 
    InventoryComponent inventory;
    EquipmentComponent equipment;
    BankComponent bank;
    StateComponent state;
    RegenerationComponent regeneration;

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
    
    // Llamado por el servidor cada tick - GAMELOOP - (delega en RegenerationComponent)
    void update(float deltaSeconds);

    /* GETTERS/SETTERS de atributos que expone */ 
    uint32_t getId() const { return this->id; }
    std::string getName() const { return this->name; }
    void setPosition(const Position& newPos) { this->pos = newPos; }

    /* IMPLEMENTACIÓN DE LA INTERFAZ ATTACKABLE */
    void receiveDamage(int amount) override {}
    bool isDead() const override { return this->stats.getHp() <= 0; }
    uint16_t getStrength() const override { return this->stats.getStrength(); }
    uint16_t getAgility() const override { return this->stats.getAgility(); }
    Position getPosition() const override { return this->pos; }

    bool canEngageInCombatWith(const Attackable& other) const;
    uint16_t getLevel() const override { stats.getLevel();}

    // Acceso a los componentes
    // Stats
    StatsComponent& getStats() { return this->stats; }
    const StatsComponent& getStats() const { return this->stats;}
    void addExperience(uint32_t amount) { stats.addExperience(amount); }
    uint16_t getHp() const { stats.getHp();}
    uint16_t getMaxHp() const { stats.getMaxHp();}

    // Inventory
    InventoryComponent& getInventory() { return this->inventory; }
    const InventoryComponent& getInventory() const { return this->inventory; }

    // Equipment
    EquipmentComponent& getEquipment() { return this->equipment; }
    const EquipmentComponent& getEquipment() const { return this->equipment; }
    int getDefense() const {return this->equipment.calculateCurrentDefense();}

    // Bank
    BankComponent& getBank() { return this->bank; }
    
    // State
    StateComponent& getState() { return this->state; }
    const StateComponent& getState() const { return this->state; }    
    bool canAttack() const { return this->state.canAttack();}
    void handleDeath();
};

#endif

