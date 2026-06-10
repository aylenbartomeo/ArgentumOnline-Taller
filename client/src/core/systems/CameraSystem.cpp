#include "CameraSystem.h"

#include <algorithm>

#include "../common/GameConstants.h"

namespace GC = GameConstants;

CameraOffset CameraSystem::compute(uint32_t myId, const SnapshotDTO& snapshot,
                                   const std::unordered_map<uint32_t, CharacterAnimator>& animators,
                                   const TileMap& map) const {
    int focusX = 0;
    int focusY = 0;

    auto ait = animators.find(myId);
    if (ait != animators.end()) {
        focusX = static_cast<int>(ait->second.getVirtualX() * GC::TILE_SIZE) + GC::TILE_SIZE / 2;
        focusY = static_cast<int>(ait->second.getVirtualY() * GC::TILE_SIZE) + GC::TILE_SIZE / 2;
    } else {
        auto it = std::find_if(snapshot.players.begin(), snapshot.players.end(),
                               [myId](const EntityDTO& e) { return e.id == myId; });
        if (it != snapshot.players.end()) {
            focusX = it->x * GC::TILE_SIZE + GC::TILE_SIZE / 2;
            focusY = it->y * GC::TILE_SIZE + GC::TILE_SIZE / 2;
        }
    }

    CameraOffset cam =
            computeCameraOffset(focusX, focusY, GC::VIEW_W, GC::VIEW_H,
                                map.getWidth() * GC::TILE_SIZE, map.getHeight() * GC::TILE_SIZE);
    cam.x -= GC::VIEW_X;
    cam.y -= GC::VIEW_Y;
    return cam;
}
