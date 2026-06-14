#ifndef NPC_STOCK_LOADER_H
#define NPC_STOCK_LOADER_H

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <toml++/toml.hpp>

class NpcStockLoader {
public:
    static std::unordered_map<uint32_t, int> loadStock(const std::filesystem::path& configPath) {
        std::unordered_map<uint32_t, int> stock;

        try {
            toml::table config = toml::parse_file(configPath.string());
            if (auto stockArray = config["stock"].as_array()) {
                for (auto&& node: *stockArray) {
                    if (auto itemTable = node.as_table()) {
                        auto itemIdOpt = (*itemTable)["item_id"].value<int64_t>();
                        auto amountOpt = (*itemTable)["amount"].value<int64_t>();

                        if (itemIdOpt && amountOpt) {
                            uint32_t itemId = static_cast<uint32_t>(*itemIdOpt);
                            int amount = static_cast<int>(*amountOpt);
                            stock[itemId] = amount;
                        }
                    }
                }
            }
        } catch (const toml::parse_error& error) {
            throw std::runtime_error("Could not parse NPC stock TOML config (" +
                                     configPath.string() + "): " + std::string(error.what()));
        }

        return stock;
    }
};

#endif  // NPC_STOCK_LOADER_H
