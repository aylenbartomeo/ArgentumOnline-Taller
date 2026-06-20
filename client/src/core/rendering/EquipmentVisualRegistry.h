#ifndef EQUIPMENT_VISUAL_REGISTRY_H
#define EQUIPMENT_VISUAL_REGISTRY_H

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "../animation/CharacterAnimator.h"

enum class WeaponAnimKind : uint8_t { GRIP, CUSTOM };

struct WeaponFacingFrames {
    int row = 0;
    int frames = 1;
};

struct WeaponAnimSpec {
    uint32_t itemId = 0;
    std::string sheet;
    WeaponAnimKind kind = WeaponAnimKind::GRIP;

    int offsetX = 0;
    int offsetY = 0;

    int pivotX = 0;
    int pivotY = 0;

    int cellW = 0;
    int cellH = 0;
    float strideX = 0.f;
    float strideY = 0.f;
    std::array<WeaponFacingFrames, 4> facings{};
};

struct ArmorVisualSpec {
    uint32_t itemId = 0;
    std::string sheet;
    std::string sheetCompact;
};

struct HelmetVisualSpec {
    uint32_t itemId = 0;
    std::string sheet;
    int frameW = 0;
    int frameH = 0;
};

struct FacingOffset {
    int x = 0;
    int y = 0;
};

struct ShieldVisualSpec {
    uint32_t itemId = 0;
    std::string sheet;
    std::array<FacingOffset, 4> facingOffset{};
};

struct RaceArmorVisualConfig {
    int cellW = 25;
    int stride = 48;
    int trimLeft = 0;
    int extraRight = 0;
    int offsetX = 0;
    int offsetY = 0;
    int walkDown = 0;
    int walkUp = 0;
    int walkLeft = 0;
    int walkRight = 0;
    bool usesCompactArmor = false;
};

struct RaceHelmetVisualConfig {
    int offsetX = 0;
    int offsetY = 0;
    int walkDown = 0;
    int walkUp = 0;
    int walkLeft = 0;
    int walkRight = 0;
};

struct HeadFacingDelta {
    int idleDx = 0;
    int moveDx = 0;
};

struct RaceHeadVisualConfig {
    int extraOffsetY = 0;
    int trimSrcXOnUp = 0;
    int trimRenderWOnUp = 0;
    std::array<HeadFacingDelta, 4> facingDelta{};
};

struct RaceShieldConfig {
    int offsetX = 0;
    int offsetY = 0;

    int walkDown = 0;
    int walkUp = 0;
    int walkLeft = 0;
    int walkRight = 0;
};

struct RaceVisualConfig {
    uint8_t entityTypeId = 0;
    std::string name;
    RaceArmorVisualConfig armor;
    RaceHelmetVisualConfig helmet;
    RaceHeadVisualConfig head;
    RaceShieldConfig shield;
};

class EquipmentVisualRegistry {
public:
    void loadFromDir(const std::string& resourcesDir);

    const WeaponAnimSpec* findWeapon(uint32_t itemId) const;
    const ArmorVisualSpec* findArmor(uint32_t itemId) const;
    const HelmetVisualSpec* findHelmet(uint32_t itemId) const;
    const ShieldVisualSpec* findShield(uint32_t itemId) const;
    const RaceVisualConfig& raceConfig(uint8_t entityTypeId) const;

private:
    void loadWeapons(const std::string& path);
    void loadArmors(const std::string& path);
    void loadHelmets(const std::string& path);
    void loadShields(const std::string& path);
    void loadRaces(const std::string& path);

    std::unordered_map<uint32_t, WeaponAnimSpec> weapons;
    std::unordered_map<uint32_t, ArmorVisualSpec> armors;
    std::unordered_map<uint32_t, HelmetVisualSpec> helmets;
    std::unordered_map<uint32_t, ShieldVisualSpec> shields;
    std::unordered_map<uint8_t, RaceVisualConfig> races;
    RaceVisualConfig fallbackRace;
};

#endif
