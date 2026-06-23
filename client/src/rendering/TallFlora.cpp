#include "TallFlora.h"

namespace {
constexpr int TREE_SPRITE_ID = 10;
constexpr int PALM_SPRITE_ID = 41;
constexpr int CACTUS_SPRITE_ID = 136;
}  // namespace

bool isTallFlora(int decorationSpriteId) {
    return decorationSpriteId == TREE_SPRITE_ID || decorationSpriteId == PALM_SPRITE_ID ||
           decorationSpriteId == CACTUS_SPRITE_ID;
}
