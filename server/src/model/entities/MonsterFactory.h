#ifndef SERVER_SRC_MODEL_ENTITIES_MONSTERFACTORY_H
#define SERVER_SRC_MODEL_ENTITIES_MONSTERFACTORY_H

#include <cstdint>
#include <unordered_map>

#include "server/src/config/MonsterConfig.h"
#include "server/src/model/entities/monster.h"

class MonsterFactory {
private:
    MonsterConfigs configs;
    FormulaEngine& formulas;

public:
    MonsterFactory(MonsterConfigs configs, FormulaEngine& formulas);

    Monster create(uint32_t id, NPCType type, Position position) const;
};

#endif
