#include "GroundItemLayer.h"

bool GroundItemLayer::placeItem(const Position& pos, uint32_t itemId, uint16_t amount) {
    groundItems[pos] = GroundItem{itemId, amount};
    return true;
}

std::optional<GroundItem> GroundItemLayer::pickUpItem(const Position& pos) {
    auto it = groundItems.find(pos);
    if (it != groundItems.end()) {
        GroundItem item = it->second;
        groundItems.erase(it);
        return item;
    }
    return std::nullopt;
}

std::optional<GroundItem> GroundItemLayer::inspectItem(const Position& pos) const {
    auto it = groundItems.find(pos);
    if (it != groundItems.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool GroundItemLayer::hasItemAt(const Position& pos) const {
    return groundItems.find(pos) != groundItems.end();
}

const std::unordered_map<Position, GroundItem, PositionHash>& GroundItemLayer::getAllItems() const {
    return groundItems;
}

void GroundItemLayer::clear() {
    groundItems.clear();
}
