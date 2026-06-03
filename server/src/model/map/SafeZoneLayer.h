#ifndef SAFE_ZONE_LAYER_H
#define SAFE_ZONE_LAYER_H

#include <string>
#include <vector>

struct SafeZone {
    std::string name;  // Ej: "Ullathorpe"
    int x, y;
    int width, height;

    bool contains(float posX, float posY) const {
        return posX >= x && posX <= x + width && posY >= y && posY <= y + height;
    }
};

class SafeZoneLayer {
private:
    std::vector<SafeZone> zones;

public:
    SafeZoneLayer() = default;
    ~SafeZoneLayer() = default;

    void addZone(const std::string& name, int x, int y, int width, int height);
    bool isSafeZone(float posX, float posY) const;
    std::string getZoneName(float posX, float posY) const;
    const std::vector<SafeZone>& getZones() const;
    void clear();
};

#endif  // SAFE_ZONE_LAYER_H
