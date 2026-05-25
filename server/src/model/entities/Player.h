#ifndef PLAYER_MOCK_H
#define PLAYER_MOCK_H

#include <cstdint>
#include <string>

#include "../../../../common/include/dto/CommandDTO.h"
#include "../combat/CombatManager.h"
#include "../components/BankComponent.h"
#include "../components/EquipmentComponent.h"
#include "../components/InventoryComponent.h"
#include "../components/RegenerationComponent.h"
#include "../components/StateComponent.h"
#include "../components/StatsComponent.h"
#include "../interfaces/Attackable.h"

#include "position.h"

class Player: public Attackable {
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
    Player(uint32_t id, const std::string& name, const RaceConfig& race,
           const CharacterClassConfig& characterClass, const PlayerConfig& playerBase);

    // Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
    // de valores random
    Player(uint32_t id, const std::string& name, const RaceConfig& race,
           const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
           const FormulaEngine& testEngine);

    // Llamado por el servidor cada tick - GAMELOOP - (delega en RegenerationComponent)
    void update(float deltaSeconds);

    /* GETTERS/SETTERS de atributos que expone */
    uint32_t getId() const { return this->id; }
    std::string getName() const { return this->name; }
    void setPosition(const Position& newPos) { this->pos = newPos; }

    /* IMPLEMENTACIÓN DE LA INTERFAZ ATTACKABLE */
    void receiveDamage(int amount) override;
    bool isDead() const override { return this->stats.getHp() == 0; }
    uint16_t getStrength() const override { return stats.getStrength(); }
    uint16_t getIntelligence() const override { return stats.getIntelligence(); }
    uint16_t getAgility() const override { return this->stats.getAgility(); }
    Position getPosition() const override { return this->pos; }

    bool canEngageInCombatWith(const Attackable& other) const override;
    uint16_t getLevel() const override { return stats.getLevel(); }

    // Acceso a los componentes
    // Stats
    StatsComponent& getStats() { return this->stats; }
    const StatsComponent& getStats() const { return this->stats; }
    void addExperience(uint32_t amount) { stats.addExperience(amount); }
    uint16_t getHp() const { return stats.getHp(); }
    uint16_t getMaxHp() const override { return stats.getMaxHp(); }
    uint16_t getMana() const override { return stats.getMana(); }
    bool consumeMana(int amount) override {
        return stats.consumeMana(static_cast<uint16_t>(amount));
    }

    // Inventory
    InventoryComponent& getInventory() { return this->inventory; }
    const InventoryComponent& getInventory() const { return this->inventory; }

    // Equipment
    EquipmentComponent& getEquipment() { return this->equipment; }
    const EquipmentComponent& getEquipment() const { return this->equipment; }
    Weapon* getEquippedWeapon() { return this->equipment.getEquippedWeapon(); }
    int getDefense() const override { return this->equipment.calculateCurrentDefense(); }

    // Bank
    BankComponent& getBank() { return this->bank; }

    // State
    StateComponent& getState() { return this->state; }
    const StateComponent& getState() const { return this->state; }
    bool canAttack() const { return this->state.canAttack(); }
    bool canBeAttacked() const override { return this->state.canBeAttacked(); }
    void handleDeath() override;

    // Se llama cada vez que el jugador inicia una accion activa en el mundo
    void onActionStarted();

    // Calcula la posición resultante de moverse en una dirección,
    // sin modificar la posición actual del jugador.
    Position tryMove(Movement direction) const;
};

#endif
