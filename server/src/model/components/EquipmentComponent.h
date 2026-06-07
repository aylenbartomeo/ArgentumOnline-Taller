#ifndef EQUIPMENT_COMPONENT_H
#define EQUIPMENT_COMPONENT_H

#include <cstdint>
#include <optional>

class Item;
class BodyArmor;
class Helmet;
class Shield;
class Weapon;

class EquipmentComponent {
private:
    const BodyArmor* bodyArmor;
    const Helmet* helmet;
    const Shield* shield;
    const Weapon* weapon;

    std::optional<uint8_t> bodyArmorSlot;
    std::optional<uint8_t> helmetSlot;
    std::optional<uint8_t> shieldSlot;
    std::optional<uint8_t> weaponSlot;

public:
    EquipmentComponent();
    ~EquipmentComponent() = default;

    // Bloqueamos copia para evitar duplicar ítems equipados por error
    EquipmentComponent(const EquipmentComponent&) = delete;
    EquipmentComponent& operator=(const EquipmentComponent&) = delete;
    EquipmentComponent(EquipmentComponent&&) = default;
    EquipmentComponent& operator=(EquipmentComponent&&) = default;

    // ========================================================================
    // LOGICA DE EQUIPAMIENTO
    // ========================================================================

    // Intenta equipar un ítem genérico derivando a la categoría correcta.
    // Retorna true si fue exitoso
    bool equipItem(const Item* item, uint8_t slotIndex);

    // Métodos específicos por slot.
    void equipBodyArmor(const BodyArmor* armor, uint8_t slotIndex);
    void equipHelmet(const Helmet* helmet, uint8_t slotIndex);
    void equipShield(const Shield* shield, uint8_t slotIndex);
    void equipWeapon(const Weapon* weapon, uint8_t slotIndex);

    // Desequipa si el slot dado está equipado en alguna parte
    void unequipSlot(uint8_t slotIndex);

    // Verifica si un slot particular del inventario está actualmente equipado
    bool isSlotEquipped(uint8_t slotIndex) const;

    // Métodos para desequipar de forma explícita
    void unequip_body_armor();
    void unequip_helmet();
    void unequip_shield();
    void unequip_weapon();

    // ========================================================================
    // GETTERS / CONSULTAS (Marcados con 'const' para tus Snapshots y World)
    // ========================================================================
    const BodyArmor* getBodyArmor() const { return bodyArmor; }
    const Helmet* getHelmet() const { return helmet; }
    const Shield* getShield() const { return shield; }
    const Weapon* getWeapon() const { return weapon; }
    Weapon* getEquippedWeapon() const { return const_cast<Weapon*>(this->weapon); }

    // Calcula la defensa total acumulada sumando la defensa de la armadura, casco y escudo
    uint16_t calculateCurrentDefense() const;
};

#endif  // EQUIPMENT_COMPONENT_H
