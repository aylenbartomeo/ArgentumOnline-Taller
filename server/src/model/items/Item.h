#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <stdexcept>

class EquipmentComponent;

class Item {
private:
    int id;
    std::string name;
    int price;

public:
    Item(int id, std::string name, int price) : id(id),
        name(std::move(name)),
        price(price) {}
    virtual ~Item() = default;

    virtual int getId() const { return id;}
    virtual const std::string& getName() const {return name;}
    int getPrice() const {return price;}

    virtual bool is_wearable() const { return false; }

    virtual uint32_t equip_on(EquipmentComponent& equipment) const {
        (void)equipment;
        return 0;
    }
};
