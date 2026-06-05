#ifndef GROUND_ITEM_LAYER_H
#define GROUND_ITEM_LAYER_H

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../../persistence/WorldPersistData.h"

#include "position.h"

// Representa un stack de items en una posición del suelo.
// Referencia al ItemRegistry por itemId (no posee el Item).
struct GroundItem {
    uint32_t itemId;  // Referencia al catálogo (ItemRegistry)
    uint16_t amount;  // Cantidad apilada
};

// Hash para usar Position como key en unordered_map
struct PositionHash {
    std::size_t operator()(const Position& pos) const {
        return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.y) << 16);
    }
};

class GroundItemLayer {
private:
    // 1 item stack por tile (regla del AO original)
    std::unordered_map<Position, GroundItem, PositionHash> groundItems;

public:
    GroundItemLayer() = default;
    ~GroundItemLayer() = default;

    bool placeItem(const Position& pos, uint32_t itemId, uint16_t amount);
    std::optional<GroundItem> pickUpItem(const Position& pos);
    std::optional<GroundItem> inspectItem(const Position& pos) const;
    bool hasItemAt(const Position& pos) const;

    // Para generar snapshots — Map lo usa internamente
    const std::unordered_map<Position, GroundItem, PositionHash>& getAllItems() const;

    std::vector<GroundItemPersistData> toPersistData() const;
    void fromPersistData(const std::vector<GroundItemPersistData>& data);

    void clear();
};

#endif  // GROUND_ITEM_LAYER_H
