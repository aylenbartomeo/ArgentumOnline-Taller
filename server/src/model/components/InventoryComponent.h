#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "../../common/include/dto/PlayerStatsDTO.h"
#include "../config/InventoryConfig.h"

class EquipmentComponent;
class ItemRegistry;

// Representa un casillero individual dentro de la grilla del inventario.
struct Slot {
    static constexpr uint16_t MAX_STACK_SIZE = 99;

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
    explicit InventoryComponent(const InventoryConfig& config);
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
    // return cantidad de items sobrantes que no se pudieron guardar (0 si entró todo).
    uint16_t addItem(uint32_t item_id, uint16_t amount, bool stackable = true);

    // Vacía todos los slots del inventario y retorna los ítems que había.
    std::vector<Slot> dropAllItems();

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

    // -- Restauracion desde persistencia --
    void setGold(uint32_t amount) { gold = std::min(amount, max_gold); }

    // Carga un slot directamente por índice (usado solo al restaurar desde disco)
    void restoreSlot(uint8_t index, uint32_t item_id, uint16_t amount) {
        if (index >= slots.size() || item_id == 0 || amount == 0)
            return;
        slots[index].item_id = item_id;
        slots[index].amount = amount;
    }

    // ========================================================================
    // SERIALIZACIÓN / INSPECCIÓN EXTERNA
    // ========================================================================
    // Devuelve todos los casilleros que estén en uso actualmente
    const std::vector<Slot>& getSlots() const { return slots; }

    // Retorna la lista de DTOs para enviar por red
    std::vector<InventorySlotDTO> getInventoryDTO(const EquipmentComponent& equipment,
                                                  const ItemRegistry* registry = nullptr) const;
    uint8_t getSize() const { return static_cast<uint8_t>(slots.size()); }
};

#endif  // INVENTORY_COMPONENT_H
