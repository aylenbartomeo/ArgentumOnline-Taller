#ifndef RESURRECTION_SERVICE_H
#define RESURRECTION_SERVICE_H

#include <cstdint>
#include <string>
#include <vector>

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
    static constexpr int DELAY_MS_FACTOR = 200;

    struct PendingResurrection {
        uint32_t playerDbId;
        float remainingTimeMs;
        Position targetPos;
    };
    std::vector<PendingResurrection> pending;

public:
    ResurrectResult requestResurrection(uint32_t dbId, const Position& playerPos, bool isDead,
                                        const std::vector<Position>& priestPositions);

    std::vector<CompletedResurrection> tick(float deltaTimeMs);
    int getDelayFactor() const { return DELAY_MS_FACTOR; }
};

#endif  // RESURRECTION_SERVICE_H
