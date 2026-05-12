#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>

#include "../utils/position.h"
#include "../utils/types.h"
#include "interfaces/MagicUser.h"
#include "interfaces/combatant.h"
#include "interfaces/interactable.h"
#include "model/FormulaEngine.h"
#include "server/src/config/CharacterConfig.h"
#include "server/src/model/inventory/inventory.h"
#include "server/src/model/items/Equipment.h"
#include "server/src/model/items/ItemRegistry.h"

class CombatManager;

enum class PlayerState {
    Alive,
    Ghost,
    Meditating,
};

class Player: public Combatant, public Interactable, public MagicUser {
private:
    // Identidad y Posición
    uint32_t id;
    std::string name;
    Race race;
    CharacterClass char_class;
    Position pos;
    Equipment equipment;

    // --- Atributos de Rol (Influyen en fórmulas) ---
    int strength;      // Fuerza (Daño)
    int intelligence;  // Inteligencia (Maná)
    int agility;       // Agilidad (Esquivar)
    int constitution;  // Constitución (Vida)

    // --- Estados Dinámicos ---
    int health;
    int max_health;  // Calculado por Constitución + Clase + Nivel
    int mana;
    int max_mana;  // Calculado por Inteligencia + Clase + Nivel
    uint32_t gold;
    uint32_t max_gold;  // Limitado por el Nivel

    uint32_t experience;
    uint16_t level;

    const ItemRegistry& item_registry;
    Inventory inventory;
    CombatManager& combat_manager;
    bool can_use_magic;
    bool can_meditate;
    float recovery_factor;
    float meditation_factor;
    PlayerState state;

    void recoverHealth(int amount);
    void recoverMana(int amount);
    void becomeGhost();

public:
    Player(uint32_t id, const std::string& name, Race race, CharacterClass char_class, Position pos,
           CombatManager& combat_manager, const PlayerConfig& playerConfig,
           const RaceConfig& raceConfig, const CharacterClassConfig& classConfig,
           const InventoryConfig& inv_config, const ItemRegistry& item_registry);

    /* Métodos de Combatant */
    void receive_damage(int amount) override;
    void attack(Combatant& target) override;
    bool is_dead() const override;
    Position get_position() const override;
    PlayerState getState() const;
    bool isMeditating() const;
    bool isGhost() const;
    bool startMeditating();
    void stopMeditating();
    void recoverOverTime(float secondsElapsed);
    void recoverMeditating(float secondsElapsed);
    bool healHealth(int amount);
    bool recoverManaAmount(int amount);
    bool restoreHealthAndMana();
    bool resurrect(Position respawnPosition);

    /* Llamaria adentro a los metodos utilizados con los ciudadanos */
    void interact(Interactable& interactable, const std::string& action,
                  const std::vector<std::string>& params) override;

    uint16_t get_strength() const override;
    uint16_t get_intelligence() const override;
    int get_mana() const override;
    void consume_mana(int amount) override;
    bool canUseMagic() const override;
    bool canMeditate() const;

    Equipment& getEquipment();
    const Equipment& getEquipment() const;

    /* ACCIONES DE INVENTARIO Y EQUIPAMIENTO */

    // Procesa el comando de red para equipar un ítem desde un slot (OpCode 0x05).
    bool equip_from_slot(uint8_t slot_index);

    // Suelta un ítem al suelo (OpCode 0x04).
    bool drop_item_to_map(uint8_t slot_index, uint16_t amount);

    /* METODOS PARA USAR CON LOS NPCS CIUDADANOS */

    // Transacciones de Comercio
    bool buy_item(uint32_t item_id, uint16_t amount, uint32_t total_price);
    bool sell_item(uint8_t slot_index, uint16_t amount, uint32_t unit_price);

    // Transacciones de Sacerdote
    void heal();
    void resurrect();

    // Transacciones de Banquero
    bool deposit_gold(uint32_t amount);
    bool withdraw_gold(uint32_t amount);
    bool deposit_item(uint8_t inv_slot, uint16_t amount);
    bool withdraw_item(uint32_t item_id, uint16_t amount);
};

#endif
