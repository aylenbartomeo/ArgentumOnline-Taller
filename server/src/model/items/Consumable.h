#pragma once
#include "Item.h"
#include "model/entities/Player.h"
#include <algorithm>
#include <stdexcept>

enum class ConsumableType {
    HEALTH,
    MANA,
    BOOST_STR,
    BOOST_AGI
};

class Consumable : public Item {
private:
    ConsumableType type;
    uint32_t durationMs;
    uint16_t effectValue;
public:
    Consumable(int id, std::string name, int price,
        ConsumableType type, int duration, int value) :
        Item(id, std::move(name), price), type(type),
        durationMs(static_cast<uint32_t>(duration)), effectValue(static_cast<uint16_t>(value)) {
        if (duration < 0) {
            throw std::invalid_argument("Consumable duration cannot be negative");
        }
        if (value < 0) {
            throw std::invalid_argument("Consumable effect value cannot be negative");
        }
    }

    bool use(Player& player) const {
        if (player.isDead()) {
                return false;
        }
	
        switch (type) {
            case ConsumableType::HEALTH: {
                if (player.getHp() == player.getMaxHp()) return false;
                
                uint16_t newHp = std::min<uint16_t>(player.getHp() + effectValue, player.getMaxHp());
                player.setHp(newHp);
                return true;
            }
            case ConsumableType::MANA: {
                if (player.getMana() == player.getMaxMana()) return false;
                
                uint16_t newMana = std::min<uint16_t>(player.getMana() + effectValue, player.getMaxMana());
                player.setMana(newMana);
                return true;
            }
            case ConsumableType::BOOST_STR: {
                player.applyBoost(BoostType::STRENGTH, static_cast<uint8_t>(effectValue), durationMs);
                return true;
            }
            case ConsumableType::BOOST_AGI: {
                player.applyBoost(BoostType::AGILITY, static_cast<uint8_t>(effectValue), durationMs);
                return true;
            }
        }
        return false;
    }

    ConsumableType getConsumableType() const { return type; }
    uint32_t getDurationMs() const { return durationMs; }
    uint16_t getEffectValue() const { return effectValue; }
};
