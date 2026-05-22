#ifndef SERVER_SRC_MODEL_ENTITIES_MONSTERFACTORY_H
#define SERVER_SRC_MODEL_ENTITIES_MONSTERFACTORY_H

#include <cstdint>
#include <unordered_map>
#include <memory>
#include "server/src/config/MonsterConfig.h"
#include "server/src/model/entities/Monster.h"

class MonsterFactory {
private:
    MonsterConfigs configs;

public:
    explicit MonsterFactory(MonsterConfigs configs);

    std::unique_ptr<Monster> create(uint32_t id, NPCType type, Position position) const;
};

#endif
