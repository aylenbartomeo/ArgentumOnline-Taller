#ifndef MAGIC_USER_H
#define MAGIC_USER_H

#include <cstdint>

class MagicUser {
public:
    virtual ~MagicUser() noexcept = default;

    virtual uint16_t get_intelligence() const = 0;
    virtual int get_mana() const = 0;
    virtual void consume_mana(int amount) = 0;
    virtual bool canUseMagic() const = 0;
};

#endif
