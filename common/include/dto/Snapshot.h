#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <cstdint>
#include <string>
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

enum class EntityAction : uint8_t {
    NONE = 0,
    WALKING = 1,
    GRABBING = 2,
    ATTACKING = 3,
    DRINKING_POTION = 4
};

// Lo mínimo y necesario para dibujar algo en el mapa
struct EntityDTO {
    uint32_t id = 0;
    EntityType type = EntityType::PLAYER;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t current_hp = 0;
    uint16_t max_hp = 0;
    uint8_t entityTypeId = 0;  // Race para players, NPCType para monsters
    uint8_t action = 0;        // EntityAction

    // Equipamiento visual (solo relevante para players, 0 = nada equipado)
    uint16_t weaponItemId = 0;
    uint16_t helmetItemId = 0;
    uint16_t shieldItemId = 0;
    uint16_t bodyArmorItemId = 0;
    uint16_t level = 0;
    uint8_t stateId = 0;
    std::string name = "";

    EntityDTO() = default;
    EntityDTO(uint32_t id, EntityType type, uint16_t x, uint16_t y, uint16_t hp, uint16_t m_hp,
              uint8_t entityType, uint8_t action = 0, uint16_t weapon = 0, uint16_t helmet = 0,
              uint16_t shield = 0, uint16_t armor = 0, uint16_t lvl = 0, uint8_t stateId = 0,
              const std::string& name = ""):
            id(id),
            type(type),
            x(x),
            y(y),
            current_hp(hp),
            max_hp(m_hp),
            entityTypeId(entityType),
            action(action),
            weaponItemId(weapon),
            helmetItemId(helmet),
            shieldItemId(shield),
            bodyArmorItemId(armor),
            level(lvl),
            stateId(stateId),
            name(name) {}
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
