#include "ResurrectionService.h"

#include <limits>

ResurrectResult ResurrectionService::requestResurrection(
        uint32_t dbId, const Position& playerPos, bool isDead,
        const std::vector<Position>& priestPositions) {
    if (!isDead) {
        return {false, "No eres un fantasma."};
    }

    if (priestPositions.empty()) {
        return {false, "No hay sacerdotes en este mundo para resucitarte."};
    }

    int minDistance = std::numeric_limits<int>::max();
    Position bestPos = priestPositions[0];

    for (const auto& pos: priestPositions) {
        int dist = playerPos.distance_to(pos);
        if (dist < minDistance) {
            minDistance = dist;
            bestPos = pos;
        }
    }

    int delayMs = minDistance * 200;
    pending.push_back({dbId, static_cast<float>(delayMs), bestPos});

    return {true,
            "Resucitando... Por favor espera " + std::to_string(delayMs / 1000) + " segundos."};
}

std::vector<CompletedResurrection> ResurrectionService::tick(float deltaTimeMs) {
    std::vector<CompletedResurrection> completed;

    for (auto it = pending.begin(); it != pending.end();) {
        it->remainingTimeMs -= deltaTimeMs;
        if (it->remainingTimeMs <= 0.0f) {
            completed.push_back({it->playerDbId, it->targetPos});
            it = pending.erase(it);
        } else {
            ++it;
        }
    }

    return completed;
}
