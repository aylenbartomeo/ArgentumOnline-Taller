#include "Player.h"

#include <algorithm>

#include "../combat/CombatManager.h"
#include "model/FormulaEngine.h"
#include "server/src/model/inventory/inventory.h"

Player::Player(uint32_t id, const std::string& name, Race race, CharacterClass char_class,
               Position pos, CombatManager& combat_manager, const PlayerConfig& playerConfig,
               const RaceConfig& raceConfig, const CharacterClassConfig& classConfig,
               const InventoryConfig& inv_config, const ItemRegistry& item_registry):
        id(id),
        name(name),
        race(race),
        char_class(char_class),
        pos(pos),
        equipment(),
        strength(playerConfig.baseStrength),
        intelligence(playerConfig.baseIntelligence),
        agility(playerConfig.baseAgility),
        constitution(playerConfig.baseConstitution),
        health(0),
        max_health(0),
        mana(0),
        max_mana(0),
        gold(playerConfig.startingGold),
        max_gold(0),
        experience(playerConfig.startingExperience),
        level(playerConfig.startingLevel),
        item_registry(item_registry),
        inventory(inv_config, FormulaEngine::getInstance().calculate_safe_gold_limit(
                                      playerConfig.startingLevel)),
        combat_manager(combat_manager),
        can_use_magic(classConfig.canUseMagic),
        can_meditate(classConfig.canUseMagic && classConfig.meditationFactor > 0.0f),
        recovery_factor(raceConfig.recoveryFactor),
        meditation_factor(classConfig.meditationFactor),
        state(PlayerState::ALIVE) {

    this->max_health = FormulaEngine::getInstance().calculate_max_life(
            static_cast<uint16_t>(this->constitution), classConfig.lifeFactor,
            raceConfig.lifeFactor, this->level);
    this->health = this->max_health;

    this->max_mana = this->can_use_magic ?
                             FormulaEngine::getInstance().calculate_max_mana(
                                     static_cast<uint16_t>(this->intelligence),
                                     classConfig.manaFactor, raceConfig.manaFactor, this->level) :
                             0;
    this->mana = this->max_mana;

    this->max_gold = FormulaEngine::getInstance().calculate_safe_gold_limit(this->level);

    this->inventory.add_gold(playerConfig.startingGold);
}

void Player::receiveDamage(int amount) {
    if (this->isGhost()) {
        return;
    }

    this->stopMeditating();

    // 1. Probabilidad de Evadir
    if (FormulaEngine::getInstance().is_attack_eluded(static_cast<uint16_t>(this->agility))) {
        // El ataque falló, no restamos nada
        return;
    }

    // 2. Mitigación por defensa equipada.
    int def = this->equipment.getDefense();

    // 3. Aplicar daño final
    int final_damage = std::max(0, amount - static_cast<int>(def));

    if (final_damage >= this->health) {
        this->health = 0;
    } else {
        this->health -= final_damage;
    }

    if (this->health == 0) {
        this->becomeGhost();
    }
}

void Player::attack(Combatant& target) {
    if (this->isGhost()) {
        return;
    }

    this->stopMeditating();

    const Weapon* equipped_weapon = this->equipment.getWeapon();
    if (equipped_weapon == nullptr) {
        return;
    }

    // Ejecutar el ataque
    bool attack_success = this->combat_manager.executeAttack(*equipped_weapon, *this, target);

    // Si el ataque fue exitoso y el objetivo muere, damos la XP
    if (attack_success && target.isDead()) {
        // Lógica de ganar experiencia...
    }
}

bool Player::isDead() const { return health == 0; }

Position Player::getPosition() const { return this->pos; }

void Player::setPosition(const Position& newPos) { this->pos = newPos; }

PlayerState Player::getState() const { return this->state; }

bool Player::isMeditating() const { return this->state == PlayerState::MEDITATING; }

bool Player::isGhost() const { return this->state == PlayerState::GHOST; }

bool Player::startMeditating() {
    if (this->state != PlayerState::ALIVE || !this->can_meditate || this->mana >= this->max_mana) {
        return false;
    }

    this->state = PlayerState::MEDITATING;
    return true;
}

void Player::stopMeditating() {
    if (this->state == PlayerState::MEDITATING) {
        this->state = PlayerState::ALIVE;
    }
}

void Player::recoverOverTime(float secondsElapsed) {
    if (this->isGhost() || secondsElapsed <= 0.0f) {
        return;
    }

    const uint16_t recoveredHealth = FormulaEngine::getInstance().calculate_passive_recovery(
            this->recovery_factor, secondsElapsed);
    this->recoverHealth(static_cast<int>(recoveredHealth));

    if (!this->isMeditating() && this->can_use_magic) {
        const uint16_t recoveredMana = FormulaEngine::getInstance().calculate_passive_recovery(
                this->recovery_factor, secondsElapsed);
        this->recoverMana(static_cast<int>(recoveredMana));
    }
}

void Player::recoverMeditating(float secondsElapsed) {
    if (!this->isMeditating() || secondsElapsed <= 0.0f) {
        return;
    }

    const uint16_t recoveredMana = FormulaEngine::getInstance().calculate_meditation_recovery(
            this->meditation_factor, static_cast<uint16_t>(this->intelligence), secondsElapsed);
    this->recoverMana(static_cast<int>(recoveredMana));

    if (this->mana >= this->max_mana) {
        this->stopMeditating();
    }
}

bool Player::healHealth(uint16_t amount) {
    if (this->isGhost() || amount == 0 || this->health >= this->max_health) {
        return false;
    }

    this->stopMeditating();
    this->recoverHealth(amount);
    return true;
}

bool Player::recoverManaAmount(int amount) {
    if (this->isGhost() || amount <= 0 || !this->can_use_magic || this->mana >= this->max_mana) {
        return false;
    }

    this->stopMeditating();
    this->recoverMana(amount);
    return true;
}

bool Player::restoreHealthAndMana() {
    if (this->isGhost()) {
        return false;
    }

    const bool hadMissingHealth = this->health < this->max_health;
    const bool hadMissingMana = this->can_use_magic && this->mana < this->max_mana;

    if (!hadMissingHealth && !hadMissingMana) {
        return false;
    }

    this->stopMeditating();
    this->health = this->max_health;

    if (this->can_use_magic) {
        this->mana = this->max_mana;
    }

    return true;
}

bool Player::resurrect(Position respawnPosition) {
    if (!this->isGhost()) {
        return false;
    }

    this->pos = respawnPosition;
    this->state = PlayerState::ALIVE;
    this->health = this->max_health;

    if (this->can_use_magic) {
        this->mana = this->max_mana;
    }

    return true;
}

void Player::interact(Interactable& interactable, const std::string& action,
                      const std::vector<std::string>& params) {
    this->stopMeditating();

    (void)interactable;
    (void)action;
    (void)params;
    // Aca deberia ocurrir la interaccion con el NPC de ciudad, dependiendo del tipo de NPC y la
    // accion, se llamaria a los metodos correspondientes (buy, sell, heal, respawn, etc). Habria
    // que generar la comunicacion dado que ambos son Interactables
}

// ============================================================================
// TRANSACCIONES DE COMERCIANTE
// ============================================================================

bool Player::buy_item(uint32_t item_id, uint16_t amount, uint32_t total_price) {
    if (!this->item_registry.get_item(item_id))
        return false;

    if (!this->inventory.remove_gold(total_price)) {
        return false;
    }

    if (!this->inventory.add_item(item_id, amount)) {
        this->inventory.add_gold(total_price);
        return false;
    }
    return true;
}

bool Player::sell_item(uint8_t slot_index, uint16_t amount, uint32_t unit_price) {
    uint16_t removed = this->inventory.remove_item(slot_index, amount);
    if (removed == 0)
        return false;
    uint32_t gold_gained = static_cast<uint32_t>(removed) * unit_price;
    this->inventory.add_gold(gold_gained);
    return true;
}

// ============================================================================
// TRANSACCIONES DE SACERDOTE
// ============================================================================

void Player::heal() { this->restoreHealthAndMana(); }

void Player::resurrect() { this->resurrect(this->pos); }

// ============================================================================
// TRANSACCIONES DE BANQUERO (Oro)
// ============================================================================

bool Player::deposit_gold(uint32_t amount) {
    if (amount == 0 || this->isGhost())
        return false;
    return this->inventory.remove_gold(amount);
}

bool Player::withdraw_gold(
        uint32_t amount) {  // Validar desde el banquero que se tiene el saldo suficiente..
    if (amount == 0 || this->isGhost())
        return false;
    this->inventory.add_gold(amount);
    return true;
}

// ============================================================================
// TRANSACCIONES DE BANQUERO (Ítems)
// ============================================================================

bool Player::deposit_item(uint8_t inv_slot, uint16_t amount) {
    if (amount == 0 || this->isGhost())
        return false;
    uint16_t removed = this->inventory.remove_item(inv_slot, amount);
    return removed > 0;
}

bool Player::withdraw_item(uint32_t item_id, uint16_t amount) {
    if (amount == 0 || this->isGhost())
        return false;
    return this->inventory.add_item(item_id, amount);
}

uint16_t Player::getStrength() const { return this->strength; }
uint16_t Player::get_intelligence() const { return this->intelligence; }
int Player::get_mana() const { return this->mana; }
uint16_t Player::getHp() const { return this->health; }
uint16_t Player::getMaxHp() const { return this->max_health; }
void Player::consume_mana(int amount) { this->mana = std::max(0, this->mana - amount); }
bool Player::canUseMagic() const { return this->can_use_magic; }
bool Player::canMeditate() const { return this->can_meditate; }

Equipment& Player::getEquipment() { return this->equipment; }

const Equipment& Player::getEquipment() const { return this->equipment; }

void Player::recoverHealth(uint16_t amount) {
    if (amount == 0) {
        return;
    }

    // this->health = std::min(this->max_health, this->health + amount);
}

void Player::recoverMana(int amount) {
    if (amount <= 0 || !this->can_use_magic) {
        return;
    }

    this->mana = std::min(this->max_mana, this->mana + amount);
}

void Player::becomeGhost() {
    this->health = 0;
    this->mana = 0;
    this->state = PlayerState::GHOST;
}

bool Player::equip_from_slot(uint8_t slot_index) {
    auto maybe_slot = this->inventory.inspect_slot(slot_index);
    if (!maybe_slot.has_value()) {
        return false;
    }

    uint32_t item_id = maybe_slot->item_id;

    const Item* item = this->item_registry.get_item(item_id);
    if (!item || !item->is_wearable()) {
        return false;
    }

    uint32_t unequipped_item_id = this->equipment.equip_item(item);
    this->inventory.remove_item(slot_index, 1);

    if (unequipped_item_id != 0) {
        this->inventory.add_item(unequipped_item_id, 1);
    }

    return true;
}

bool Player::drop_item_to_map(uint8_t slot_index, uint16_t amount) {
    auto maybe_slot = this->inventory.inspect_slot(slot_index);
    if (!maybe_slot.has_value()) {
        return false;
    }

    /*uint32_t item_id = maybe_slot->item_id;*/

    uint16_t removed = this->inventory.remove_item(slot_index, amount);
    if (removed == 0) {
        return false;
    }

    // Notificar al mapa para instanciar la entidad visual 'Drop' en nuestra coordenada actual
    // Descomentar/Ajustar cuando esté integrado el subsistema del mapa en el jugador.
    // Ejemplo conceptual:
    // this->current_map.create_drop(item_id, removed, this->pos);

    return true;
}

void Player::setSkin(int skinId) { (void)skinId; }
