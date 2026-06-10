#ifndef GROUND_ITEM_LABEL_H
#define GROUND_ITEM_LABEL_H

#include <cstdint>
#include <optional>
#include <string>

inline std::optional<std::string> groundAmountLabel(uint16_t amount) {
    if (amount > 1) {
        return std::to_string(amount);
    }
    return std::nullopt;
}

#endif
