#include "EquipmentVisualRegistry.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace {

using json = nlohmann::json;

constexpr int FACING_COUNT = 4;
constexpr const char* FACING_KEYS[FACING_COUNT] = {"down", "up", "left", "right"};

json loadJsonFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[EquipmentVisualRegistry] NO SE PUDO ABRIR: " << path << std::endl;
        return json::object();
    }
    try {
        json data;
        f >> data;
        return data;
    } catch (const json::parse_error& e) {
        std::cerr << "[EquipmentVisualRegistry] JSON invalido en " << path << ": " << e.what()
                  << std::endl;
        return json::object();
    }
}

int intOr(const json& obj, const char* key, int def) { return obj.value(key, def); }
float floatOr(const json& obj, const char* key, float def) { return obj.value(key, def); }
std::string strOr(const json& obj, const char* key, const std::string& def = "") {
    return obj.value(key, def);
}

WeaponFacingFrames parseWeaponFacingFrames(const json& obj) {
    WeaponFacingFrames f;
    f.row = intOr(obj, "row", 0);
    f.frames = intOr(obj, "frames", 1);
    return f;
}

FacingOffset parseFacingOffset(const json& obj) {
    FacingOffset o;
    o.x = intOr(obj, "x", 0);
    o.y = intOr(obj, "y", 0);
    return o;
}

HeadFacingDelta parseHeadFacingDelta(const json& obj) {
    HeadFacingDelta d;
    d.idleDx = intOr(obj, "idleDx", 0);
    d.moveDx = intOr(obj, "moveDx", 0);
    return d;
}

WeaponAnimSpec parseWeapon(const json& obj) {
    WeaponAnimSpec spec;
    spec.itemId = obj.at("itemId").get<uint32_t>();
    spec.sheet = strOr(obj, "sheet");
    spec.kind = (strOr(obj, "kind", "grip") == "custom") ? WeaponAnimKind::CUSTOM :
                                                           WeaponAnimKind::GRIP;
    spec.offsetX = intOr(obj, "offsetX", 0);
    spec.offsetY = intOr(obj, "offsetY", 0);

    if (spec.kind == WeaponAnimKind::GRIP) {
        const json& pivot = obj.at("pivot");
        spec.pivotX = intOr(pivot, "x", 0);
        spec.pivotY = intOr(pivot, "y", 0);
    } else {
        spec.cellW = intOr(obj, "cellW", 0);
        spec.cellH = intOr(obj, "cellH", 0);
        spec.strideX = floatOr(obj, "strideX", 0.f);
        spec.strideY = floatOr(obj, "strideY", 0.f);

        const json& facings = obj.at("facings");
        for (int i = 0; i < FACING_COUNT; ++i)
            spec.facings[i] = parseWeaponFacingFrames(facings.at(FACING_KEYS[i]));
    }
    return spec;
}

ArmorVisualSpec parseArmor(const json& obj) {
    ArmorVisualSpec spec;
    spec.itemId = obj.at("itemId").get<uint32_t>();
    spec.sheet = strOr(obj, "sheet");
    spec.sheetCompact = strOr(obj, "sheetCompact");
    return spec;
}

HelmetVisualSpec parseHelmet(const json& obj) {
    HelmetVisualSpec spec;
    spec.itemId = obj.at("itemId").get<uint32_t>();
    spec.sheet = strOr(obj, "sheet");
    spec.frameW = intOr(obj, "frameW", 0);
    spec.frameH = intOr(obj, "frameH", 0);
    return spec;
}

ShieldVisualSpec parseShield(const json& obj) {
    ShieldVisualSpec spec;
    spec.itemId = obj.at("itemId").get<uint32_t>();
    spec.sheet = strOr(obj, "sheet");

    const json& offsets = obj.at("facingOffset");
    for (int i = 0; i < FACING_COUNT; ++i)
        spec.facingOffset[i] = parseFacingOffset(offsets.at(FACING_KEYS[i]));
    return spec;
}

RaceArmorVisualConfig parseRaceArmor(const json& obj) {
    RaceArmorVisualConfig cfg;
    cfg.cellW = intOr(obj, "cellW", cfg.cellW);
    cfg.stride = intOr(obj, "stride", cfg.stride);
    cfg.trimLeft = intOr(obj, "trimLeft", cfg.trimLeft);
    cfg.extraRight = intOr(obj, "extraRight", cfg.extraRight);
    cfg.offsetX = intOr(obj, "offsetX", cfg.offsetX);
    cfg.offsetY = intOr(obj, "offsetY", cfg.offsetY);
    cfg.walkDown = intOr(obj, "walkDown", cfg.walkDown);
    cfg.walkUp = intOr(obj, "walkUp", cfg.walkUp);
    cfg.walkLeft = intOr(obj, "walkLeft", cfg.walkLeft);
    cfg.walkRight = intOr(obj, "walkRight", cfg.walkRight);
    cfg.usesCompactArmor = obj.value("usesCompactArmor", false);
    return cfg;
}

RaceHelmetVisualConfig parseRaceHelmet(const json& obj) {
    RaceHelmetVisualConfig cfg;
    cfg.offsetX = intOr(obj, "offsetX", cfg.offsetX);
    cfg.offsetY = intOr(obj, "offsetY", cfg.offsetY);
    cfg.walkDown = intOr(obj, "walkDown", cfg.walkDown);
    cfg.walkUp = intOr(obj, "walkUp", cfg.walkUp);
    cfg.walkLeft = intOr(obj, "walkLeft", cfg.walkLeft);
    cfg.walkRight = intOr(obj, "walkRight", cfg.walkRight);
    return cfg;
}

RaceHeadVisualConfig parseRaceHead(const json& obj) {
    RaceHeadVisualConfig cfg;
    cfg.extraOffsetY = intOr(obj, "extraOffsetY", 0);
    cfg.trimSrcXOnUp = intOr(obj, "trimSrcXOnUp", 0);
    cfg.trimRenderWOnUp = intOr(obj, "trimRenderWOnUp", 0);

    const json& deltas = obj.at("facingDelta");
    for (int i = 0; i < FACING_COUNT; ++i)
        cfg.facingDelta[i] = parseHeadFacingDelta(deltas.at(FACING_KEYS[i]));
    return cfg;
}

}  // namespace

void EquipmentVisualRegistry::loadFromDir(const std::string& resourcesDir) {
    const std::string base = resourcesDir + "data/equipment/";
    loadWeapons(base + "weapons.json");
    loadArmors(base + "armors.json");
    loadHelmets(base + "helmets.json");
    loadShields(base + "shields.json");
    loadRaces(base + "races.json");
}

void EquipmentVisualRegistry::loadWeapons(const std::string& path) {
    const json data = loadJsonFile(path);
    for (const json& entry: data.value("weapons", json::array())) {
        const WeaponAnimSpec spec = parseWeapon(entry);
        weapons[spec.itemId] = spec;
    }
}

void EquipmentVisualRegistry::loadArmors(const std::string& path) {
    const json data = loadJsonFile(path);
    for (const json& entry: data.value("armors", json::array())) {
        const ArmorVisualSpec spec = parseArmor(entry);
        armors[spec.itemId] = spec;
    }
}

void EquipmentVisualRegistry::loadHelmets(const std::string& path) {
    const json data = loadJsonFile(path);
    for (const json& entry: data.value("helmets", json::array())) {
        const HelmetVisualSpec spec = parseHelmet(entry);
        helmets[spec.itemId] = spec;
    }
}

void EquipmentVisualRegistry::loadShields(const std::string& path) {
    const json data = loadJsonFile(path);
    for (const json& entry: data.value("shields", json::array())) {
        const ShieldVisualSpec spec = parseShield(entry);
        shields[spec.itemId] = spec;
    }
}

void EquipmentVisualRegistry::loadRaces(const std::string& path) {
    const json data = loadJsonFile(path);
    for (const json& entry: data.value("races", json::array())) {
        RaceVisualConfig cfg;
        cfg.entityTypeId = static_cast<uint8_t>(entry.at("entityTypeId").get<int>());
        cfg.name = strOr(entry, "name");
        cfg.armor = parseRaceArmor(entry.at("armor"));
        cfg.helmet = parseRaceHelmet(entry.at("helmet"));
        cfg.head = parseRaceHead(entry.at("head"));

        if (entry.contains("shield")) {
            const auto& shObj = entry["shield"];
            cfg.shield.offsetX = shObj.value("offsetX", 0);
            cfg.shield.offsetY = shObj.value("offsetY", 0);
            cfg.shield.walkDown = shObj.value("walkDown", 0);
            cfg.shield.walkUp = shObj.value("walkUp", 0);
            cfg.shield.walkLeft = shObj.value("walkLeft", 0);
            cfg.shield.walkRight = shObj.value("walkRight", 0);
        }
        races[cfg.entityTypeId] = cfg;
    }
}

const WeaponAnimSpec* EquipmentVisualRegistry::findWeapon(uint32_t itemId) const {
    const auto it = weapons.find(itemId);
    return it != weapons.end() ? &it->second : nullptr;
}

const ArmorVisualSpec* EquipmentVisualRegistry::findArmor(uint32_t itemId) const {
    const auto it = armors.find(itemId);
    return it != armors.end() ? &it->second : nullptr;
}

const HelmetVisualSpec* EquipmentVisualRegistry::findHelmet(uint32_t itemId) const {
    const auto it = helmets.find(itemId);
    return it != helmets.end() ? &it->second : nullptr;
}

const ShieldVisualSpec* EquipmentVisualRegistry::findShield(uint32_t itemId) const {
    const auto it = shields.find(itemId);
    return it != shields.end() ? &it->second : nullptr;
}

const RaceVisualConfig& EquipmentVisualRegistry::raceConfig(uint8_t entityTypeId) const {
    const auto it = races.find(entityTypeId);
    return it != races.end() ? it->second : fallbackRace;
}
