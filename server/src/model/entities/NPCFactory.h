#ifndef NPC_FACTORY_H
#define NPC_FACTORY_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "../interfaces/Interactable.h"

#include "Banker.h"
#include "Merchant.h"
#include "Priest.h"

class NPCFactory {
private:
    std::unordered_map<NPCType, std::function<std::unique_ptr<Interactable>(uint32_t, Position)>>
            creators;

public:
    NPCFactory(const ItemRegistry& registry, GlobalBank& globalBank) {
        creators[NPCType::MERCHANT] = [&registry](uint32_t id, Position pos) {
            return std::make_unique<Merchant>(id, pos, registry);
        };
        creators[NPCType::PRIEST] = [&registry](uint32_t id, Position pos) {
            return std::make_unique<Priest>(id, pos, registry);
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
