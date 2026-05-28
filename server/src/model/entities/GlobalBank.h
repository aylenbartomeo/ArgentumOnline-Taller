#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include "items/Item.h"

// El banco es global: todos los Bankers del mundo comparten esta instancia
class GlobalBank {
private:
    // Por jugador: su oro y sus ítems depositados
    struct BankAccount {
        uint32_t gold = 0;
        std::vector<std::unique_ptr<Item>> items;
    };
    std::unordered_map<uint32_t, BankAccount> accounts;  // playerId → cuenta

public:
    void depositGold(uint32_t playerId, uint32_t amount) {
        accounts[playerId].gold += amount;
    }

    bool withdrawGold(uint32_t playerId, uint32_t amount) {
        auto it = accounts.find(playerId);
        if (it == accounts.end() || it->second.gold < amount) return false;
        it->second.gold -= amount;
        return true;
    }

    void depositItem(uint32_t playerId, std::unique_ptr<Item> item) {
        accounts[playerId].items.push_back(std::move(item));
    }

    std::unique_ptr<Item> withdrawItem(uint32_t playerId, const std::string& name) {
        auto it = accounts.find(playerId);
        if (it == accounts.end()) return nullptr;

        auto& items = it->second.items;
        for (auto itemIt = items.begin(); itemIt != items.end(); ++itemIt) {
            if ((*itemIt)->getName() == name) {
                auto item = std::move(*itemIt);
                items.erase(itemIt);
                return item;
            }
        }
        return nullptr;
    }

    std::string listContents(uint32_t playerId) const {
        auto it = accounts.find(playerId);
        if (it == accounts.end()) return "Tu cuenta está vacía.";

        std::string result = "=== Tu banco ===\n";
        result += "Oro: " + std::to_string(it->second.gold) + "\n";
        for (const auto& item : it->second.items) {
            result += "- " + item->getName() + "\n";
        }
        return result;
    }
};