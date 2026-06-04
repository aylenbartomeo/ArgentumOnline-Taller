#include "server/src/model/entities/MonsterFactory.h"

#include <stdexcept>
#include <utility>

#include "../include/FormulaEngine.h"

MonsterFactory::MonsterFactory(MonsterConfigs configs): configs(std::move(configs)) {}

std::unique_ptr<Monster> MonsterFactory::create(uint32_t id, NPCType type,
                                                Position position) const {
    auto config = configs.find(type);
    if (config == configs.end()) {
        throw std::runtime_error("Tipo de monstruo no configurado");
    }

    // Retornamos un puntero único envolviendo al nuevo objeto
    return std::make_unique<Monster>(id, type, position, config->second);
}
