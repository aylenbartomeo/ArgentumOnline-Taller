#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <cstdint>
#include <vector>

/* ACA va toda la informacion que necesite el cliente
 * para actualizar su estado, como por ejemplo:
 * - Posicion del jugador
 * - Vida del jugador
 * - Inventario del jugador
 * - Posicion de los monstruos
 * - Vida de los monstruos
 * - Posicion de los items en el mapa
 * - Cantidad de oro del jugador
 * - Estado del jugador (vivo, muerto, meditando)
 */

enum class EntityType : uint8_t { PLAYER, MONSTER, NPC };

// Lo mínimo y necesario para dibujar algo en el mapa
struct EntityDTO {
    uint32_t id = 0;
    EntityType type = EntityType::PLAYER;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t current_hp = 0;
    uint16_t max_hp = 0;
    uint16_t sprite_id = 0;  // imagen (Ej: 10=Cuerpo Desnudo, 25=Orco)

    EntityDTO() = default;
    EntityDTO(uint32_t id, EntityType type, uint16_t x, uint16_t y, uint16_t hp, uint16_t m_hp,
              uint16_t sprite):
            id(id), type(type), x(x), y(y), current_hp(hp), max_hp(m_hp), sprite_id(sprite) {}
};


struct GroundItemDTO {
    uint32_t itemId = 0;
    uint16_t amount = 0;
    uint16_t x = 0;
    uint16_t y = 0;

    GroundItemDTO() = default;
    GroundItemDTO(uint32_t itemId, uint16_t amount, uint16_t x, uint16_t y):
            itemId(itemId), amount(amount), x(x), y(y) {}
};

struct ProjectileDTO {
    uint32_t id = 0;
    float x = 0.f;
    float y = 0.f;
    float velX = 0.f;
    float velY = 0.f;
    uint16_t spriteId = 0;
};

struct SnapshotDTO {
    std::vector<EntityDTO> players;
    std::vector<EntityDTO> monsters;
    std::vector<GroundItemDTO> groundItems;
    std::vector<ProjectileDTO> projectiles;

    SnapshotDTO() = default;
};

#endif
