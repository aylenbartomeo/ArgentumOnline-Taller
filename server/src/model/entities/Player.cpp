#include "Player.h"

#include <algorithm>

#include "server/src/model/items/Item.h"
#include "server/src/model/items/ItemRegistry.h"

Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, Race race,
               CharacterClass cls, const RaceConfig& raceConfig,
               const CharacterClassConfig& classConfig, const PlayerConfig& playerBase,
               const ItemRegistry& itemRegistry, const InventoryConfig& inventoryConfig,
               const Position& spawn):
        id(entityId),
        dbId(dbId),
        name(name),
        pos(spawn),
        stats(race, cls, raceConfig, classConfig, playerBase),
        inventory(inventoryConfig),
        equipment(),
        state(),
        regeneration(stats, state, raceConfig, classConfig),
        itemRegistry(&itemRegistry) {
    this->addGold(playerBase.startingGold);
}

// Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
// de valores random
Player::Player(uint32_t entityId, uint32_t dbId, const std::string& name, Race race,
               CharacterClass charClass, const RaceConfig& raceConf,
               const CharacterClassConfig& classConf, const PlayerConfig& playerBase,
               const InventoryConfig& inventoryConfig, const FormulaEngine& testEngine):
        id(entityId),
        dbId(dbId),
        name(name),
        pos({0, 0}),
        // Respetamos el mismo orden de parámetros que en StatsComponent
        stats(race, charClass, raceConf, classConf, playerBase, testEngine),
        inventory(inventoryConfig),
        equipment(),
        state(),
        regeneration(stats, state, raceConf, classConf, testEngine),
        itemRegistry(nullptr) {
    // También agregamos el oro inicial para que los tests pasen
    this->addGold(playerBase.startingGold);
}

// Eliminado: equipItemById

bool Player::equipFromSlot(uint8_t slotIndex) {
    onActionStarted();

    // 1. Si el slot ya está equipado, el jugador quiere desequiparlo (doble clic)
    if (equipment.isSlotEquipped(slotIndex)) {
        equipment.unequipSlot(slotIndex);
        return true;
    }

    // 2. Si no estaba equipado, intentamos equiparlo
    auto slotOpt = inventory.inspectSlot(slotIndex);
    if (!slotOpt)
        return false;

    uint32_t itemId = slotOpt->item_id;
    if (!itemRegistry)
        return false;

    const Item* item = itemRegistry->get_item(static_cast<int>(itemId));
    if (!item || !item->is_wearable())
        return false;

    // si es un arma y ya hay otra equipada en un slot distinto, desequiparla primero
    const Weapon* asWeapon = dynamic_cast<const Weapon*>(item);
    if (asWeapon && equipment.getWeapon() != nullptr) {
        equipment.unequip_weapon();  // limpia weapon y weaponSlot
    }

    // Equipamos el ítem pero NO lo sacamos del inventario
    equipment.equipItem(item, slotIndex);

    return true;
}

void Player::update(float dtMs) {
    if (this->isDead()) {
        stats.clearBoosts();  // Regla AO: al morir se pierden los elixires
        currentAction = 0;
        actionTimerMs = 0.0f;
    } else if (actionTimerMs > 0.0f) {
        actionTimerMs -= dtMs;
        if (actionTimerMs <= 0.0f) {
            currentAction = 0;
            actionTimerMs = 0.0f;
        }
    }
    stats.updateTicks(dtMs);
    this->updateResurrectionTimer(dtMs);
    regeneration.tick(dtMs / 1000.0f);
}

bool Player::canEngageInCombatWith(const Attackable& other) const {
    const Player* otherPlayer = dynamic_cast<const Player*>(&other);
    if (!otherPlayer) {
        return true;  // Puede atacar/ser atacado por NPCs o monstruos sin restricciones de nivel
    }

    // Regla 1: newbie no puede atacar ni ser atacado por jugadores
    if (FormulaEngine::getInstance().isNewbie(this->stats.getLevel()) ||
        FormulaEngine::getInstance().isNewbie(otherPlayer->stats.getLevel())) {
        return false;
    }

    // Regla 2: diferencia de nivel máxima de 10
    if (!FormulaEngine::getInstance().isPvpLevelValid(this->stats.getLevel(),
                                                         otherPlayer->stats.getLevel())) {
        return false;
    }
    return true;
}

void Player::handleDeath() {
    state.die();
    stats.takeDamage(stats.getHp());
    stats.loseExperienceUponDeath();
}

void Player::resurrect() {
    stats.restoreHp();
    state.resurrect();
}

PlayerStatsDTO Player::getStatsDTO() const {
    PlayerStatsDTO dto;
    dto.currentHp = stats.getHp();
    dto.maxHp = stats.getMaxHp();
    dto.currentMana = stats.getMana();
    dto.maxMana = stats.getMaxMana();
    dto.gold = inventory.getGold();
    dto.exp = stats.getExp();
    dto.level = stats.getLevel();
    dto.expIntoLevel = stats.getExpIntoCurrentLevel();
    dto.expForLevel = stats.getExpForCurrentLevel();
    dto.race = stats.getRace();
    dto.characterClass = stats.getCharacterClass();
    dto.agilityBuffTimeLeftMs = stats.getAgilityBoostTimeLeft();
    dto.strengthBuffTimeLeftMs = stats.getStrengthBoostTimeLeft();
    dto.inventory = inventory.getInventoryDTO(equipment, itemRegistry);
    return dto;
}

EntityDTO Player::toEntityDTO() const {
    EntityDTO dto;
    dto.id = dbId;
    dto.type = EntityType::PLAYER;
    dto.name = this->name;
    dto.x = pos.x;
    dto.y = pos.y;
    dto.current_hp = stats.getHp();
    dto.max_hp = stats.getMaxHp();
    dto.entityTypeId = static_cast<uint8_t>(stats.getRace());
    dto.action = currentAction;
    dto.level = stats.getLevel();
    dto.stateId = state.isGhost() ? 1 : 0;
    // Equipamiento visual
    const Weapon* w = equipment.getWeapon();
    dto.weaponItemId = w ? static_cast<uint16_t>(w->getId()) : 0;
    const Helmet* h = equipment.getHelmet();
    dto.helmetItemId = h ? static_cast<uint16_t>(h->getId()) : 0;
    const Shield* s = equipment.getShield();
    dto.shieldItemId = s ? static_cast<uint16_t>(s->getId()) : 0;
    const BodyArmor* a = equipment.getBodyArmor();
    dto.bodyArmorItemId = a ? static_cast<uint16_t>(a->getId()) : 0;

    return dto;
}

void Player::setAction(uint8_t action, float durationMs) {
    currentAction = action;
    actionTimerMs = durationMs;
}

uint16_t Player::addInventoryItem(uint32_t item_id, uint16_t amount) {
    bool stackable = true;
    onActionStarted();
    if (this->itemRegistry != nullptr) {
        stackable = this->itemRegistry->isStackable(item_id);
    }
    return this->inventory.addItem(item_id, amount, stackable);
}

uint16_t Player::removeInventoryItem(uint8_t slot_index, uint16_t amount) {
    onActionStarted();
    uint16_t removed = this->inventory.removeItem(slot_index, amount);
    if (removed > 0) {
        auto slotOpt = inventory.inspectSlot(slot_index);
        if (!slotOpt) {
            equipment.unequipSlot(slot_index);
        }
    }
    return removed;
}

std::optional<Slot> Player::inspectInventorySlot(uint8_t slot_index) const {
    return this->inventory.inspectSlot(slot_index);
}

std::vector<Slot> Player::dropAllItems() {
    for (uint8_t i = 0; i < inventory.getSize(); ++i) {
        equipment.unequipSlot(i);
    }
    return this->inventory.dropAllItems();
}

uint32_t Player::dropExcessGold() { return this->inventory.dropExcessGold(); }
void Player::onActionStarted() { state.stopMeditating(); }

void Player::receiveDamage(int amount) {
    if (isDead())
        return;
    onActionStarted();
    if (amount < 0)
        return;
    stats.takeDamage(static_cast<uint16_t>(amount));
    if (stats.getHp() == 0) {
        handleDeath();
    }
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

void Player::immobilizeForResurrection(float durationMs) {
        this->resurrectionImmobilizedTimeMs = durationMs;
}

bool Player::isImmobilized() const {
    return this->resurrectionImmobilizedTimeMs > 0.0f;
}

void Player::updateResurrectionTimer(float deltaTimeMs) {
    if (this->resurrectionImmobilizedTimeMs > 0.0f) {
        this->resurrectionImmobilizedTimeMs -= deltaTimeMs;
        if (this->resurrectionImmobilizedTimeMs < 0.0f) {
            this->resurrectionImmobilizedTimeMs = 0.0f;
        }
    }
}

PlayerPersistData Player::toPersistData() const {
    PlayerPersistData d{};
    d.dbId = this->dbId;
    d.posX = pos.x;
    d.posY = pos.y;
    d.hp = stats.getHp();
    d.mana = stats.getMana();
    d.level = stats.getLevel();
    d.exp = stats.getExp();
    d.gold = inventory.getGold();

    // Estado
    if (state.isGhost())
        d.stateId = 1;
    else if (state.isMeditating())
        d.stateId = 2;
    else
        d.stateId = 0;

    // Raza y clase
    d.race = static_cast<uint8_t>(stats.getRace());
    d.characterClass = static_cast<uint8_t>(stats.getCharacterClass());

    // Inventario
    const auto& slots = inventory.getSlots();
    d.inventorySize = static_cast<uint8_t>(std::min(slots.size(), size_t(32)));
    for (uint8_t i = 0; i < d.inventorySize; ++i) {
        d.inventory[i].item_id = slots[i].item_id;
        d.inventory[i].amount = slots[i].amount;
    }

    d.equippedSlots = 0;
    for (uint8_t i = 0; i < d.inventorySize; ++i) {
        if (equipment.isSlotEquipped(i)) {
            d.equippedSlots |= (1u << i);
        }
    }

    return d;
}

void Player::fromPersistData(const PlayerPersistData& data) {
    stats.restoreFromPersist(data.hp, data.mana, data.exp, data.level);
    inventory.setGold(data.gold);

    uint8_t slots = std::min<uint8_t>(data.inventorySize, 32);
    for (uint8_t i = 0; i < slots; ++i) {
        inventory.restoreSlot(i, data.inventory[i].item_id, data.inventory[i].amount);
    }

    for (uint8_t i = 0; i < slots; ++i) {
        if ((data.equippedSlots >> i) & 1u) {
            equipFromSlot(i);
        }
    }

    if (data.stateId == 1)
        handleDeath();
    else if (data.stateId == 2)
        startMeditating();
}
