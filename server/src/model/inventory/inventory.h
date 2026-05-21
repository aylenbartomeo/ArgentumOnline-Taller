#ifndef INVENTORY_H_
#define INVENTORY_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "server/src/config/InventoryConfig.h"

// Representa un casillero individual dentro de la grilla del inventario.
// Almacena únicamente metadatos (ID y cantidad).
// struct Slot {
//     uint32_t item_id{0};
//     uint16_t amount{0};

//     bool is_empty() const { return item_id == 0 || amount == 0; }
//     void clear() {
//         item_id = 0;
//         amount = 0;
//     }
// };

class Inventory {
private:
    // std::vector<Slot> slots;
    // uint32_t gold{0};
    // uint32_t safe_gold_limit{0};

public:
    // explicit Inventory(const InventoryConfig& config, uint32_t initial_safe_gold);
    // ~Inventory() = default;

    // Inventory(const Inventory&) = delete;
    // Inventory& operator=(const Inventory&) = delete;
    // Inventory(Inventory&&) = default;
    // Inventory& operator=(Inventory&&) = default;

    // // ========================================================================
    // // GESTIÓN DE ÍTEMS
    // // ========================================================================

    // // Intenta añadir ítems aplicando apilamiento automático (Stacking).
    // // item_id ID del catálogo.
    // // amount Cantidad a sumar.
    // // return true si se pudo almacenar todo, false si no hay espacio disponible.
    // bool add_item(uint32_t item_id, uint16_t amount);

    // // Remueve una cantidad específica de ítems de un casillero.
    // // slot_index Índice del casillero (0 a N-1).
    // // amount Cantidad a descontar.
    // // return uint16_t Cantidad real que se logró remover.
    // uint16_t remove_item(uint8_t slot_index, uint16_t amount);

    // // Permite inspeccionar el contenido de un casillero en modo lectura.
    // // slot_index Índice del casillero.
    // // return std::optional<Slot> Datos del casillero si es válido y está ocupado.
    // std::optional<Slot> inspect_slot(uint8_t slot_index) const;

    // // ========================================================================
    // // GESTIÓN DE ORO Y ECONOMÍA
    // // ========================================================================

    // void add_gold(uint32_t amount);
    // bool remove_gold(uint32_t amount);
    // uint32_t get_gold() const { return gold; }

    // // Actualiza el tope de oro seguro (Llamado al subir de nivel).
    // void update_safe_limit(uint32_t new_limit);

    // // Aplica la penalidad por muerte: retiene el límite seguro y suelta el resto.
    // // return uint32_t Cantidad de oro en exceso que debe caer al suelo.
    // uint32_t drop_excess_gold();

    // // ========================================================================
    // // SERIALIZACIÓN (Red / Persistencia)
    // // ========================================================================
    // const std::vector<Slot>& get_slots() const { return slots; }
};

#endif  // INVENTORY_H_
