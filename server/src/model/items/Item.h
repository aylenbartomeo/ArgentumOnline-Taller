#ifndef SERVER_SRC_MODEL_ITEMS_ITEM_H
#define SERVER_SRC_MODEL_ITEMS_ITEM_H

#include <cstdint>
#include <string>

class EquipmentComponent;

class Item {
public:
    virtual ~Item() = default;

    virtual int getId() const = 0;
    virtual const std::string& getName() const = 0;

    virtual bool is_wearable() const { return false; }

    virtual uint32_t equip_on(EquipmentComponent& equipment) const {
        (void)equipment;
        return 0;
    }
    // Para cuando queramos comprar y vender un item
    virtual int getPrice() const { return 100; }
};

#endif
