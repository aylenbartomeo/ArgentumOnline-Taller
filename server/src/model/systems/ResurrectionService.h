#ifndef RESURRECTION_SERVICE_H
#define RESURRECTION_SERVICE_H

#include <vector>
#include <string>
#include <cstdint>
#include "position.h"

struct ResurrectResult {
    bool success;
    std::string message;
};

struct CompletedResurrection {
    uint32_t playerDbId;
    Position targetPos;
};

class ResurrectionService {
private:
    struct PendingResurrection {
        uint32_t playerDbId;
        float remainingTimeMs;
        Position targetPos;
    };
    std::vector<PendingResurrection> pending;

public:
    ResurrectResult requestResurrection(uint32_t dbId, const Position& playerPos,
                                         bool isDead,
                                         const std::vector<Position>& priestPositions);

    std::vector<CompletedResurrection> tick(float deltaTimeMs);
};

#endif // RESURRECTION_SERVICE_H
