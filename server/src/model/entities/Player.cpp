#include "Player.h"

#include "server/src/model/items/ItemRegistry.h"
#include "server/src/model/items/Item.h"

Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, const RaceConfig& race,
               const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
               const ItemRegistry& itemRegistry):
        id(entityId),
        dbId(dbId),
        name(name),
        pos({0, 0}),
        // Stats ahora solo maneja combate (sin max_gold)
        stats(race, characterClass, playerBase),
        // Inventario ahora absorbe la economía: 20 slots, 5000 seguro, 100000 tope máximo
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        bank(50, 999999),
        state(),
        regeneration(stats, state, race, characterClass),
        itemRegistry(&itemRegistry) {}

// Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
// de valores random
Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, const RaceConfig& race,
               const CharacterClassConfig& characterClass, const PlayerConfig& playerBase,
               const FormulaEngine& testEngine):
        id(entityId),
        dbId(dbId),
        name(name),
        pos({0, 0}),
        stats(race, characterClass, playerBase, testEngine),
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        bank(50, 999999),
        state(),
        regeneration(stats, state, race, characterClass, testEngine),
        itemRegistry(nullptr) {}

uint32_t Player::equipItemById(uint32_t itemId) {
    if (!itemRegistry) return 0;
    const Item* item = itemRegistry->get_item(static_cast<int>(itemId));
    if (!item || !item->is_wearable()) return 0;
    return equipment.equipItem(item);
}

void Player::update(float deltaSeconds) { regeneration.tick(deltaSeconds); }

bool Player::canEngageInCombatWith(const Attackable& other) const {
    // Regla 1: newbie no puede atacar ni ser atacado por jugadores
    if (FormulaEngine::getInstance().is_newbie(this->stats.getLevel())) {
        return false;
    }
    // Regla 2: diferencia de nivel máxima de 10
    // Aplica solo contra otros jugadores — Monster::canEngageInCombatWith
    // devuelve true siempre, así que esta validación solo se activa
    // cuando AMBOS son Players (ambos lados del contrato se evalúan)
    if (!FormulaEngine::getInstance().is_pvp_level_valid(this->stats.getLevel(),
                                                         other.getLevel())) {
        return false;
    }
    return true;
}

void Player::handleDeath() {
    state.die();
    // TODO: inventory->dropItems();
}

void Player::onActionStarted() { state.stopMeditating(); }

void Player::receiveDamage(int amount) {
    state.stopMeditating();
    if (amount < 0)
        return;
    stats.takeDamage(static_cast<uint16_t>(amount));
}

Position Player::tryMove(Movement direction) const {
    Position candidate = this->pos;
    switch (direction) {
        case Movement::UP:
            candidate.y -= 1;
            break;
        case Movement::DOWN:
            candidate.y += 1;
            break;
        case Movement::LEFT:
            candidate.x -= 1;
            break;
        case Movement::RIGHT:
            candidate.x += 1;
            break;
        default:
            break;
    }
    return candidate;
}

///                                                              ///
/// COSAS DEL PLAYER ANTERIOR QUE PODRIAMOS LLEGAR A IMPLEMENTAR ///
///                                                              ///

// Player::Player(uint32_t id, const std::string& name, Race race, CharacterClass char_class,
//         Position pos, CombatManager& combat_manager, const PlayerConfig& playerConfig,
//         const RaceConfig& raceConfig, const CharacterClassConfig& classConfig,
//         const InventoryConfig& inv_config, const ItemRegistry& item_registry):
//         id(id),
//         name(name),
//         race(race),
//         char_class(char_class),
//         pos(pos),
//         equipment(),
//         strength(playerConfig.baseStrength),
//         intelligence(playerConfig.baseIntelligence),
//         agility(playerConfig.baseAgility),
//         constitution(playerConfig.baseConstitution),
//         health(0),
//         max_health(0),
//         mana(0),
//         max_mana(0),
//         gold(playerConfig.startingGold),
//         max_gold(0),
//         experience(playerConfig.startingExperience),
//         level(playerConfig.startingLevel),
//         item_registry(item_registry),
//         inventory(inv_config, FormulaEngine::getInstance().calculate_safe_gold_limit(
//                                         playerConfig.startingLevel)),
//         combat_manager(combat_manager),
//         can_use_magic(classConfig.canUseMagic),
//         can_meditate(classConfig.canUseMagic && classConfig.meditationFactor > 0.0f),
//         recovery_factor(raceConfig.recoveryFactor),
//         meditation_factor(classConfig.meditationFactor),
//         state(PlayerState::ALIVE) {

//         this->max_health = FormulaEngine::getInstance().calculate_max_life(
//                 static_cast<uint16_t>(this->constitution), classConfig.lifeFactor,
//                 raceConfig.lifeFactor, this->level);
//         this->health = this->max_health;

//         this->max_mana = this->can_use_magic ?
//                                 FormulaEngine::getInstance().calculate_max_mana(
//                                         static_cast<uint16_t>(this->intelligence),
//                                         classConfig.manaFactor, raceConfig.manaFactor,
//                                         this->level) :
//                                 0;
//         this->mana = this->max_mana;

//         this->max_gold = FormulaEngine::getInstance().calculate_safe_gold_limit(this->level);

//         this->inventory.add_gold(playerConfig.startingGold);
// }

// bool Player::startMeditating() {
//     if (this->state != PlayerState::ALIVE || !this->can_meditate || this->mana >= this->max_mana)
//     {
//         return false;
//     }

//     this->state = PlayerState::MEDITATING;
//     return true;
// }

// void Player::stopMeditating() {
//     if (this->state == PlayerState::MEDITATING) {
//         this->state = PlayerState::ALIVE;
//     }
// }

// void Player::recoverOverTime(float secondsElapsed) {
//     if (this->isGhost() || secondsElapsed <= 0.0f) {
//         return;
//     }

//     const uint16_t recoveredHealth = FormulaEngine::getInstance().calculate_passive_recovery(
//             this->recovery_factor, secondsElapsed);
//     this->recoverHealth(static_cast<int>(recoveredHealth));

//     if (!this->isMeditating() && this->can_use_magic) {
//         const uint16_t recoveredMana = FormulaEngine::getInstance().calculate_passive_recovery(
//                 this->recovery_factor, secondsElapsed);
//         this->recoverMana(static_cast<int>(recoveredMana));
//     }
// }

// void Player::recoverMeditating(float secondsElapsed) {
//     if (!this->isMeditating() || secondsElapsed <= 0.0f) {
//         return;
//     }

//     const uint16_t recoveredMana = FormulaEngine::getInstance().calculate_meditation_recovery(
//             this->meditation_factor, static_cast<uint16_t>(this->intelligence), secondsElapsed);
//     this->recoverMana(static_cast<int>(recoveredMana));

//     if (this->mana >= this->max_mana) {
//         this->stopMeditating();
//     }
// }

// bool Player::healHealth(uint16_t amount) {
//     if (this->isGhost() || amount == 0 || this->health >= this->max_health) {
//         return false;
//     }

//     this->stopMeditating();
//     this->recoverHealth(amount);
//     return true;
// }

// bool Player::recoverManaAmount(int amount) {
//     if (this->isGhost() || amount <= 0 || !this->can_use_magic || this->mana >= this->max_mana) {
//         return false;
//     }

//     this->stopMeditating();
//     this->recoverMana(amount);
//     return true;
// }

// bool Player::restoreHealthAndMana() {
//     if (this->isGhost()) {
//         return false;
//     }

//     const bool hadMissingHealth = this->health < this->max_health;
//     const bool hadMissingMana = this->can_use_magic && this->mana < this->max_mana;

//     if (!hadMissingHealth && !hadMissingMana) {
//         return false;
//     }

//     this->stopMeditating();
//     this->health = this->max_health;

//     if (this->can_use_magic) {
//         this->mana = this->max_mana;
//     }

//     return true;
// }

// bool Player::resurrect(Position respawnPosition) {
//     if (!this->isGhost()) {
//         return false;
//     }

//     this->pos = respawnPosition;
//     this->state = PlayerState::ALIVE;
//     this->health = this->max_health;

//     if (this->can_use_magic) {
//         this->mana = this->max_mana;
//     }

//     return true;
// }

// ============================================================================
// TRANSACCIONES DE COMERCIANTE
// ============================================================================

// bool Player::buy_item(uint32_t item_id, uint16_t amount, uint32_t total_price) {
//     if (!this->item_registry.get_item(item_id))
//         return false;

//     if (!this->inventory.remove_gold(total_price)) {
//         return false;
//     }

//     if (!this->inventory.add_item(item_id, amount)) {
//         this->inventory.add_gold(total_price);
//         return false;
//     }
//     return true;
// }

// bool Player::sell_item(uint8_t slot_index, uint16_t amount, uint32_t unit_price) {
//     uint16_t removed = this->inventory.remove_item(slot_index, amount);
//     if (removed == 0)
//         return false;
//     uint32_t gold_gained = static_cast<uint32_t>(removed) * unit_price;
//     this->inventory.add_gold(gold_gained);
//     return true;
// }
