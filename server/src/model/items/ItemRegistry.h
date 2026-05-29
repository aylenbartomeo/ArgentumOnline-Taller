#ifndef ITEMREGISTRY_H
#define ITEMREGISTRY_H

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "server/src/model/items/Armor.h"
#include "server/src/model/items/BodyArmor.h"
#include "server/src/model/items/Helmet.h"
#include "server/src/model/items/Item.h"
#include "server/src/model/items/Shield.h"
#include "server/src/model/items/Weapon.h"

class ItemRegistry {
private:
    std::unordered_map<int, std::unique_ptr<Weapon>> weapons;
    std::unordered_map<int, std::unique_ptr<Armor>> armors;
    // Para futuros consumibles/pociones/genéricos
    std::unordered_map<int, std::unique_ptr<Item>> items;

public:
    // Inicializa el catálogo consumiendo los Loaders existentes.
    // configPath Ruta al archivo items.toml. LUEGO SE LE PASA COMO REFERENCIA A CADA UNO DE LOS
    // PLAYERS EN EL JUEGO (GameLoop o main)
    explicit ItemRegistry(const std::filesystem::path& configPath);
    ~ItemRegistry() = default;

    // Bloqueamos copias para proteger la unicidad de los punteros y datos
    ItemRegistry(const ItemRegistry&) = delete;
    ItemRegistry& operator=(const ItemRegistry&) = delete;
    ItemRegistry(ItemRegistry&&) = default;
    ItemRegistry& operator=(ItemRegistry&&) = default;

    // Obtiene la definición inmutable de un ítem genérico en O(1).
    const Item* get_item(int item_id) const;

    // Retorna el ítem casteado de forma segura a Weapon.
    const Weapon* get_weapon(int item_id) const;

    // Retorna el ítem casteado de forma segura a Armor.
    const Armor* get_armor(int item_id) const;

    // Determina si un ítem es apilable. Las armas y armaduras no lo son.
    bool isStackable(int item_id) const;
};

#endif  // ITEMREGISTRY_H
