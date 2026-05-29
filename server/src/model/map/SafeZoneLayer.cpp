#include "SafeZoneLayer.h"

#include <algorithm>

void SafeZoneLayer::addZone(const std::string& name, int x, int y, int width, int height) {
    zones.push_back(SafeZone{name, x, y, width, height});
}

bool SafeZoneLayer::isSafeZone(float posX, float posY) const {
    return std::any_of(zones.begin(), zones.end(),
                       [posX, posY](const SafeZone& zone) { return zone.contains(posX, posY); });
}

std::string SafeZoneLayer::getZoneName(float posX, float posY) const {
    auto it = std::find_if(zones.begin(), zones.end(), [posX, posY](const SafeZone& zone) {
        return zone.contains(posX, posY);
    });
    if (it != zones.end()) {
        return it->name;
    }
    return "";
}

const std::vector<SafeZone>& SafeZoneLayer::getZones() const { return zones; }

void SafeZoneLayer::clear() { zones.clear(); }
