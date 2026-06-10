#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <cstdint>
#include <unordered_map>

#include "../animation/CharacterAnimator.h"
#include "../rendering/TileMap.h"
#include "../rendering/Viewport.h"
#include "common/include/dto/Snapshot.h"

class CameraSystem {
public:
    CameraOffset compute(uint32_t myId, const SnapshotDTO& snapshot,
                         const std::unordered_map<uint32_t, CharacterAnimator>& animators,
                         const TileMap& map) const;
};

#endif
