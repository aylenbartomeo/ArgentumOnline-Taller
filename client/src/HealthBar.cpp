#include "HealthBar.h"

#include <algorithm>

namespace {
constexpr int BAR_HEIGHT = 4;
constexpr int BAR_OFFSET_Y = 36;
}  // namespace

HealthBarLayout computeHealthBar(int currentHp, int maxHp, int entityX, int entityY, int tileSize) {
    HealthBarLayout layout{};
    layout.visible = (maxHp > 0 && currentHp < maxHp);
    if (maxHp <= 0) {
        return layout;
    }
    const int clampedHp = std::clamp(currentHp, 0, maxHp);
    const int barY = entityY - BAR_OFFSET_Y;
    layout.background = BarRect{entityX, barY, tileSize, BAR_HEIGHT};
    layout.fill = BarRect{entityX, barY, tileSize * clampedHp / maxHp, BAR_HEIGHT};
    return layout;
}
