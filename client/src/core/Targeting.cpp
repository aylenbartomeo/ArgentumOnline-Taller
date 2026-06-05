#include "Targeting.h"

#include <cstdlib>

Cell screenToCell(int screenX, int screenY, int cameraX, int cameraY, int tileSize) {
    return Cell{(screenX + cameraX) / tileSize, (screenY + cameraY) / tileSize};
}

std::optional<uint32_t> pickTargetAt(int col, int row, const SnapshotDTO& snap, uint32_t selfId,
                                     int maxRange) {
    const EntityDTO* self = findEntityById(snap, selfId);
    if (self == nullptr) {
        return std::nullopt;
    }
    const int dist = std::abs(static_cast<int>(self->x) - col) +
                     std::abs(static_cast<int>(self->y) - row);
    if (dist > maxRange) {
        return std::nullopt;
    }
    for (const EntityDTO& monster: snap.monsters) {
        if (monster.current_hp == 0)
            continue;
        if (monster.id != selfId && monster.x == col && monster.y == row)
            return monster.id;
    }
    for (const EntityDTO& player: snap.players) {
        if (player.current_hp == 0)
            continue;
        if (player.id != selfId && player.x == col && player.y == row)
            return player.id;
    }
    return std::nullopt;
}

const EntityDTO* findEntityById(const SnapshotDTO& snap, uint32_t id) {
    for (const EntityDTO& monster: snap.monsters) {
        if (monster.id == id)
            return &monster;
    }
    for (const EntityDTO& player: snap.players) {
        if (player.id == id)
            return &player;
    }
    return nullptr;
}
