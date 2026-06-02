#include "Player.h"

#include "server/src/model/items/Item.h"
#include "server/src/model/items/ItemRegistry.h"

Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, Race race,
               CharacterClass cls, const RaceConfig& raceConfig,
               const CharacterClassConfig& classConfig, const PlayerConfig& playerBase,
               const ItemRegistry& itemRegistry, const Position& spawn):
        id(entityId),
        dbId(dbId),
        name(name),
        pos(spawn),
        // Stats ahora solo maneja combate (sin max_gold)
        stats(race, cls, raceConfig, classConfig, playerBase),
        // Inventario ahora absorbe la economía: 20 slots, 5000 seguro, 100000 tope máximo
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        state(),
        regeneration(stats, state, raceConfig, classConfig),
        itemRegistry(&itemRegistry) {
    // Restauramos el oro inicial que arregló el test
    this->addGold(playerBase.startingGold);
}

// Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
// de valores random
Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, Race race,
               CharacterClass charClass, const RaceConfig& raceConf,
               const CharacterClassConfig& classConf, const PlayerConfig& playerBase,
               const FormulaEngine& testEngine):
        id(entityId),
        dbId(dbId),
        name(name),
        pos({0, 0}),
        // Respetamos el mismo orden de parámetros que en StatsComponent
        stats(race, charClass, raceConf, classConf, playerBase, testEngine),
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        state(),
        regeneration(stats, state, raceConf, classConf, testEngine),
        itemRegistry(nullptr) {
    // También agregamos el oro inicial para que los tests pasen
    this->addGold(playerBase.startingGold);
}

uint32_t Player::equipItemById(uint32_t itemId) {
    if (!itemRegistry)
        return 0;
    const Item* item = itemRegistry->get_item(static_cast<int>(itemId));
    if (!item || !item->is_wearable())
        return 0;
    return equipment.equipItem(item);
}

bool Player::equipFromSlot(uint8_t slotIndex) {
    auto slotOpt = inventory.inspectSlot(slotIndex);
    if (!slotOpt)
        return false;

    uint32_t itemId = slotOpt->item_id;
    if (!itemRegistry)
        return false;

    const Item* item = itemRegistry->get_item(static_cast<int>(itemId));
    if (!item || !item->is_wearable())
        return false;

    // Equipar y recibir el ID del ítem que fue reemplazado
    uint32_t replacedId = equipment.equipItem(item);

    // Sacar el ítem del inventario (solo consumimos 1, ya que equipamos 1 a la vez)
    inventory.removeItem(slotIndex, 1);

    // Si había algo equipado antes, devolverlo al inventario
    if (replacedId != 0) {
        inventory.addItem(replacedId, 1);
    }
    return true;
}

void Player::update(float dtMs) {
    if (this->isDead()) {
        stats.clearBoosts();  // Regla AO: al morir se pierden los elixires
        return;
    }
    stats.updateTicks(dtMs);
    regeneration.tick(dtMs / 1000.0f);  // tick espera segundos, dtMs viene en ms
}

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
    // A death should also zero the HP so isDead() becomes true
    stats.takeDamage(stats.getHp());
    // TODO: inventory->dropItems();
}

void Player::resurrect() {
    stats.restoreHp();
    state.resurrect();
}

uint16_t Player::addInventoryItem(uint32_t item_id, uint16_t amount) {
    bool stackable = true;
    if (this->itemRegistry != nullptr) {
        stackable = this->itemRegistry->isStackable(item_id);
    }
    return this->inventory.addItem(item_id, amount, stackable);
}

uint16_t Player::removeInventoryItem(uint8_t slot_index, uint16_t amount) {
    return this->inventory.removeItem(slot_index, amount);
}

std::optional<Slot> Player::inspectInventorySlot(uint8_t slot_index) const {
    return this->inventory.inspectSlot(slot_index);
}

std::vector<Slot> Player::dropAllItems() { return this->inventory.dropAllItems(); }

uint32_t Player::dropExcessGold() { return this->inventory.dropExcessGold(); }
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

void Player::applyBoost(BoostType type, uint8_t value, uint32_t durationMs) {
    stats.addBoost(type, value, durationMs);
}
