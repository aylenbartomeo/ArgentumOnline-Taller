#include "server/src/model/entities/MonsterFactory.h"

#include <stdexcept>
#include <utility>

MonsterFactory::MonsterFactory(MonsterConfigs configs, FormulaEngine& formulas):
        configs(std::move(configs)), formulas(formulas) {}

Monster MonsterFactory::create(uint32_t id, NPCType type, Position position) const {
    const auto config = this->configs.find(type);
    if (config == this->configs.end()) {
        throw std::invalid_argument("Unknown monster type");
    }

    return Monster(id, type, position, this->formulas, config->second);
}
