#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include <cstdint>
#include <optional>
#include <vector>

// Representa un casillero individual dentro de la grilla del inventario.
struct Slot {
    uint32_t item_id{0};
    uint16_t amount{0};

    bool is_empty() const { return item_id == 0 || amount == 0; }
    void clear() {
        item_id = 0;
        amount = 0;
    }
};

class InventoryComponent {
private:
    std::vector<Slot> slots;   // Grilla de ítems
    uint32_t gold{0};          // Cantidad actual de oro
    uint32_t safe_gold_limit;  // Oro que no se cae al morir (p.ej. según el nivel)
    uint32_t max_gold;         // Capacidad máxima de la billetera/mochila

public:
    explicit InventoryComponent(uint8_t total_slots, uint32_t initial_safe_gold, uint32_t max_g);
    ~InventoryComponent() = default;

    // Bloqueamos copia para evitar duplicaciones accidentales de ítems en memoria
    InventoryComponent(const InventoryComponent&) = delete;
    InventoryComponent& operator=(const InventoryComponent&) = delete;
    
    InventoryComponent(InventoryComponent&&) = default;
    InventoryComponent& operator=(InventoryComponent&&) = default;

    // ========================================================================
    // GESTIÓN DE ÍTEMS
    // ========================================================================

    // Intenta añadir ítems aplicando apilamiento automático (Stacking).
    // return true si se pudo almacenar todo, false si no hay espacio disponible.
    bool addItem(uint32_t item_id, uint16_t amount);

    // Remueve una cantidad específica de ítems de un casillero.
    // return Cantidad real que se logró remover.
    uint16_t removeItem(uint8_t slot_index, uint16_t amount);

    // Permite inspeccionar el contenido de un casillero en modo lectura.
    // El método es 'const' para que el World o las Snapshots puedan leerlo sin problemas
    std::optional<Slot> inspectSlot(uint8_t slot_index) const;

    // ========================================================================
    // GESTIÓN DE ORO Y ECONOMÍA
    // ========================================================================

    bool addGold(uint32_t amount);
    bool removeGold(uint32_t amount);
    uint32_t getGold() const { return gold; }

    // Actualiza el tope de oro seguro (Llamado al subir de nivel).
    void updateSafeLimit(uint32_t new_limit);

    // Aplica la penalidad por muerte: retiene el límite seguro y suelta el resto.
    // return Cantidad de oro en exceso que debe caer al suelo.
    uint32_t dropExcessGold();

    // ========================================================================
    // SERIALIZACIÓN / INSPECCIÓN EXTERNA
    // ========================================================================
    const std::vector<Slot>& getSlots() const { return slots; }
    uint8_t getSize() const { return static_cast<uint8_t>(slots.size()); }
};

#endif  // INVENTORY_COMPONENT_H
