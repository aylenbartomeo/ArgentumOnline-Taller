#ifndef NPC_FACTORY_H
#define NPC_FACTORY_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "../interfaces/Interactable.h"
#include "../config/NpcStockLoader.h"
#include "Banker.h"
#include "Merchant.h"
#include "Priest.h"

class NPCFactory {
private:
    std::unordered_map<NPCType, std::function<std::unique_ptr<Interactable>(uint32_t, Position)>>
            creators;
    std::unordered_map<uint32_t, int> merchantInitialStock;
    std::unordered_map<uint32_t, int> priestInitialStock;
public:
    NPCFactory(const ItemRegistry& registry, GlobalBank& globalBank) {
        std::string merchantPath = "config/stockMerchant.toml";
        if (!std::filesystem::exists(merchantPath)) merchantPath = "../../config/stockMerchant.toml";
        std::string priestPath = "config/stockPriest.toml";
        if (!std::filesystem::exists(priestPath)) priestPath = "../../config/stockPriest.toml";

        try {
            merchantInitialStock = NpcStockLoader::loadStock(merchantPath);
            priestInitialStock = NpcStockLoader::loadStock(priestPath);
        } catch (const std::exception& e) {
            std::cerr << "[ERROR Factory] No se pudo cargar la configuración de stocks: " << e.what() << std::endl;
        }

        creators[NPCType::MERCHANT] = [this, &registry](uint32_t id, Position pos) {
            auto merchant = std::make_unique<Merchant>(id, pos, registry);
            merchant->initializeStock(merchantInitialStock);
            return merchant;
        };
        creators[NPCType::PRIEST] = [this, &registry](uint32_t id, Position pos) {
            auto priest = std::make_unique<Priest>(id, pos, registry);
            priest->initializeStock(priestInitialStock);
            return priest;
        };
        creators[NPCType::BANKER] = [&globalBank, &registry](uint32_t id, Position pos) {
            return std::make_unique<Banker>(id, pos, globalBank, registry);
        };
    }

    std::unique_ptr<Interactable> create(uint32_t id, NPCType type, Position pos) const {
        auto it = creators.find(type);
        if (it != creators.end()) {
            return it->second(id, pos);
        }
        return nullptr;
    }
};

#endif
