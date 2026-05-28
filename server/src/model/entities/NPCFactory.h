#ifndef NPC_FACTORY_H
#define NPC_FACTORY_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "Citizen.h"
#include "Merchant.h"
#include "Priest.h"
#include "banker.h"

class NPCFactory {
private:
    std::unordered_map<NPCType, std::function<std::unique_ptr<Citizen>(uint32_t, Position)>> creators;

public:
    NPCFactory() {
        creators[NPCType::MERCHANT] = [](uint32_t id, Position pos) { return std::make_unique<Merchant>(id, pos); };
        creators[NPCType::PRIEST] = [](uint32_t id, Position pos) { return std::make_unique<Priest>(id, pos); };
        creators[NPCType::BANKER] = [](uint32_t id, Position pos) { return std::make_unique<Banker>(id, pos); };
    }

    std::unique_ptr<Citizen> create(uint32_t id, NPCType type, Position pos) const {
        auto it = creators.find(type);
        if (it != creators.end()) {
            return it->second(id, pos);
        }
        return nullptr;
    }
};

#endif
