#include "server/src/model/entities/MonsterFactory.h"

#include <stdexcept>
#include <utility>

#include "../include/model/FormulaEngine.h"

MonsterFactory::MonsterFactory(MonsterConfigs configs): configs(std::move(configs)) {}

Monster MonsterFactory::create(uint32_t id, NPCType type, Position position) const {
    const auto config = this->configs.find(type);
    if (config == this->configs.end()) {
        throw std::invalid_argument("Unknown monster type");
    }

    return Monster(id, type, position, config->second);
}
