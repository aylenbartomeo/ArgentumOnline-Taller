#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

class EquipmentComponent;

class Item {
private:
    int id;
    std::string name;
    int price;

public:
    Item(int id, std::string name, int price): id(id), name(std::move(name)), price(price) {}
    virtual ~Item() = default;

    virtual int getId() const { return id; }
    virtual const std::string& getName() const { return name; }
    int getPrice() const { return price; }
    virtual bool isMagic() const = 0;
    virtual bool is_wearable() const { return false; }

    virtual void equip_on(EquipmentComponent& equipment, uint8_t slotIndex) const {
        (void)equipment;
        (void)slotIndex;
    }
};
