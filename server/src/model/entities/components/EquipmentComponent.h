#ifndef EQUIPMENT_COMPONENT_H
#define EQUIPMENT_COMPONENT_H

#include <cstdint>

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
    // Devuelve el ID del ítem que fue reemplazado (para devolverlo al inventario), o 0 si no había nada.
    uint32_t equipItem(const Item* item);

    // Métodos específicos por slot. Devuelven el ID del ítem desequipado (si lo hubiera).
    uint32_t equipBodyArmor(const BodyArmor* armor);
    uint32_t equipHelmet(const Helmet* helmet);
    uint32_t equipShield(const Shield* shield);
    uint32_t equipWeapon(const Weapon* weapon);

    // Métodos para desequipar de forma explícita
    uint32_t unequip_body_armor();
    uint32_t unequip_helmet();
    uint32_t unequip_shield();
    uint32_t unequip_weapon();

    // ========================================================================
    // GETTERS / CONSULTAS (Marcados con 'const' para tus Snapshots y World)
    // ========================================================================
    const BodyArmor* getBodyArmor() const { return bodyArmor; }
    const Helmet* getHelmet() const { return helmet; }
    const Shield* getShield() const { return shield; }
    const Weapon* getWeapon() const { return weapon; }

    // Calcula la defensa total acumulada sumando la defensa de la armadura, casco y escudo
    int getDefense() const;
};

#endif  // EQUIPMENT_COMPONENT_H
