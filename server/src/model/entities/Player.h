#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../../../common/include/dto/CommandDTO.h"
#include "../combat/CombatManager.h"
#include "../components/EquipmentComponent.h"
#include "../components/InventoryComponent.h"
#include "../components/RegenerationComponent.h"
#include "../components/StateComponent.h"
#include "../components/StatsComponent.h"
#include "../interfaces/Attackable.h"
#include "../interfaces/Interactable.h"

#include "position.h"

class ItemRegistry;

class Player: public Attackable {
private:
    uint32_t id;    // Entity ID de runtime
    uint32_t dbId;  // Database ID persistente de AuthManager
    std::string name;
    Position pos;
    StatsComponent stats;
    InventoryComponent inventory;
    EquipmentComponent equipment;
    StateComponent state;
    RegenerationComponent regeneration;
    const ItemRegistry* itemRegistry;  // Puntero para permitir nullptr en tests
public:
    Player(uint32_t entityId, uint32_t dbId, const std::string& name, const RaceConfig& race,
           const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
           const ItemRegistry& itemRegistry, const Position& spawn);

    // Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
    // de valores random (no requiere ItemRegistry)
    Player(uint32_t entityId, uint32_t dbId, const std::string& name, const RaceConfig& race,
           const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
           const FormulaEngine& testEngine);

    // Llamado por el servidor cada tick - GAMELOOP - (delega en RegenerationComponent)
    void update(float deltaSeconds);

    // Equipa un ítem resolviendo su ID contra el registry
    uint32_t equipItemById(uint32_t itemId);

    // Equipa un ítem directamente desde un slot del inventario.
    // Retorna true si pudo equiparlo, false en caso contrario.
    bool equipFromSlot(uint8_t slotIndex);

    // Se llama cada vez que el jugador inicia una accion activa en el mundo
    void onActionStarted();

    // Calcula la posición resultante de moverse en una dirección,
    // sin modificar la posición actual del jugador.
    Position tryMove(Movement direction) const;

    /* GETTERS/SETTERS de atributos que expone */
    uint32_t getId() const { return this->id; }
    uint32_t getDbId() const { return this->dbId; }
    std::string getName() const override { return this->name; }
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

    /* Acceso a los componentes */
    // Stats
    StatsComponent& getStats() { return this->stats; }
    const StatsComponent& getStats() const { return this->stats; }
    void addExperience(uint32_t amount) { stats.addExperience(amount); }
    uint16_t getHp() const { return stats.getHp(); }
    uint16_t getMaxHp() const override { return stats.getMaxHp(); }
    uint16_t getMana() const { return stats.getMana(); }
    uint16_t getMaxMana() const { return stats.getMaxMana(); }
    bool consumeMana(int amount) { return stats.consumeMana(static_cast<uint16_t>(amount)); }
    void restoreHp() { stats.restoreHp(); }
    void restoreMana() { stats.restoreMana(); }
    void setHp(uint16_t newHp) { stats.setHp(newHp); }
    void setMana(uint16_t newMana) { stats.setMana(newMana); }
    void applyBoost(BoostType type, uint8_t value, uint32_t durationMs);

    // Inventory
    InventoryComponent& getInventory() { return this->inventory; }
    const InventoryComponent& getInventory() const { return this->inventory; }
    bool addGold(uint32_t amount) { return inventory.addGold(amount); }
    bool removeGold(uint32_t amount) { return inventory.removeGold(amount); }
    uint32_t getGold() const { return inventory.getGold(); }
    bool addItem(uint32_t item_id, uint16_t amount) {
        return inventory.addItem(item_id, amount) == 0;
    }
    uint16_t removeItem(uint8_t slot_index, uint16_t amount) {
        return inventory.removeItem(slot_index, amount);
    }
    uint8_t getSize() const { return inventory.getSize(); }
    std::optional<Slot> inspectSlot(uint8_t slot_index) const {
        return inventory.inspectSlot(slot_index);
    }

    uint16_t addInventoryItem(uint32_t item_id, uint16_t amount);
    uint16_t removeInventoryItem(uint8_t slot_index, uint16_t amount);
    std::optional<Slot> inspectInventorySlot(uint8_t slot_index) const;
    std::vector<Slot> dropAllItems();
    uint32_t dropExcessGold();

    // Equipment
    EquipmentComponent& getEquipment() { return this->equipment; }
    const EquipmentComponent& getEquipment() const { return this->equipment; }
    Weapon* getEquippedWeapon() { return this->equipment.getEquippedWeapon(); }
    int getDefense() const override { return this->equipment.calculateCurrentDefense(); }

    // State
    StateComponent& getState() { return this->state; }
    const StateComponent& getState() const { return this->state; }
    bool canAttack() const { return this->state.canAttack(); }
    bool canBeAttacked() const override { return this->state.canBeAttacked(); }
    void handleDeath() override;
    void resurrect();
};

#endif
