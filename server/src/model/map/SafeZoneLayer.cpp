#include "SafeZoneLayer.h"

void SafeZoneLayer::addZone(const std::string& name, int x, int y, int width, int height) {
    zones.push_back(SafeZone{name, x, y, width, height});
}

bool SafeZoneLayer::isSafeZone(float posX, float posY) const {
    for (const auto& zone : zones) {
        if (zone.contains(posX, posY)) {
            return true;
        }
    }
    return false;
}

std::string SafeZoneLayer::getZoneName(float posX, float posY) const {
    for (const auto& zone : zones) {
        if (zone.contains(posX, posY)) {
            return zone.name;
        }
    }
    return "";
}

const std::vector<SafeZone>& SafeZoneLayer::getZones() const {
    return zones;
}

void SafeZoneLayer::clear() {
    zones.clear();
}
