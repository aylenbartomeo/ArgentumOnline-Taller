#include "Player.h"

#include "server/src/model/items/Item.h"
#include "server/src/model/items/ItemRegistry.h"

Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, Race race,
               CharacterClass charClass, const RaceConfig& raceConf,
               const CharacterClassConfig& classConf, const PlayerConfig& playerBase,
               const ItemRegistry& itemRegistry, const Position& spawn):
        id(entityId),
        dbId(dbId),
        name(name),
        pos(spawn),
        // Pasar race y charClass al constructor de StatsComponent
        stats(raceConf, classConf, playerBase, race, charClass),
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        state(),
        regeneration(stats, state, raceConf, classConf),
        itemRegistry(&itemRegistry) {}

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
        // Pasar race y charClass junto con el testEngine
        stats(raceConf, classConf, playerBase, race, charClass, testEngine),
        inventory(InventoryConfig{20, 100000}, 5000),
        equipment(),
        state(),
        regeneration(stats, state, raceConf, classConf, testEngine),
        itemRegistry(nullptr) {}

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

PlayerPersistData Player::toPersistData() const {
    PlayerPersistData data{};
    data.dbId = this->dbId;
    data.posX = this->pos.x;
    data.posY = this->pos.y;

    // Estadísticas
    data.level = this->stats.getLevel();
    data.exp = this->stats.getExp();
    data.hp = this->stats.getHp();
    data.maxHp = this->stats.getMaxHp();
    data.mana = this->stats.getMana();
    data.maxMana = this->stats.getMaxMana();

    // Economía
    data.gold = this->inventory.getGold();

    // Identidad y Estado
    data.race = static_cast<uint32_t>(this->stats.getRace());
    data.charClass = static_cast<uint32_t>(this->stats.getCharClass());
    data.isGhost = this->state.isGhost();

    // Inventario
    const auto& slots = this->inventory.getSlots();
    for (size_t i = 0; i < slots.size() && i < MAX_INVENTORY_SLOTS; ++i) {
        data.inventory[i].itemId = slots[i].item_id;
        data.inventory[i].amount = slots[i].amount;
    }

    // Equipamiento
    data.equippedWeapon = equipment.getWeapon() ? equipment.getWeapon()->getId() : 0;
    data.equippedArmor = equipment.getBodyArmor() ? equipment.getBodyArmor()->getId() : 0;
    data.equippedShield = equipment.getShield() ? equipment.getShield()->getId() : 0;
    data.equippedHelmet = equipment.getHelmet() ? equipment.getHelmet()->getId() : 0;

    return data;
}

void Player::restoreFromPersistData(const PlayerPersistData& data) {
    // 1. Stats
    this->stats.restoreFromPersist(data.hp, data.mana, data.exp, data.level);

    // 2. Economía
    this->inventory.setGold(data.gold);

    // 3. Estado (Si estaba muerto al desconectarse, lo matamos)
    if (data.isGhost) {
        this->state.die();
    }

    // 4. Inventario
    for (size_t i = 0; i < MAX_INVENTORY_SLOTS; ++i) {
        if (data.inventory[i].itemId != 0 && data.inventory[i].amount > 0) {
            this->inventory.restoreSlot(i, data.inventory[i].itemId, data.inventory[i].amount);
        }
    }

    // 5. Equipamiento (Pasamos los IDs directamente al registry)
    if (data.equippedWeapon != 0)
        this->equipItemById(data.equippedWeapon);
    if (data.equippedArmor != 0)
        this->equipItemById(data.equippedArmor);
    if (data.equippedShield != 0)
        this->equipItemById(data.equippedShield);
    if (data.equippedHelmet != 0)
        this->equipItemById(data.equippedHelmet);
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
