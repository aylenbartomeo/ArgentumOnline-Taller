#include "Targeting.h"

#include <algorithm>
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
    const int dist =
            std::abs(static_cast<int>(self->x) - col) + std::abs(static_cast<int>(self->y) - row);
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
    auto m_it = std::find_if(snap.monsters.begin(), snap.monsters.end(),
                             [id](const EntityDTO& m) { return m.id == id; });
    if (m_it != snap.monsters.end()) {
        return &(*m_it);
    }

    auto p_it = std::find_if(snap.players.begin(), snap.players.end(),
                             [id](const EntityDTO& p) { return p.id == id; });
    if (p_it != snap.players.end()) {
        return &(*p_it);
    }

    return nullptr;
}
