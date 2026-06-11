#ifndef FX_TYPE_H
#define FX_TYPE_H

#include <cstdint>

enum class FxType : uint8_t {
    DEFAULT,
    SWORD,
    EXPLOSION,
    NUDOSO_IMPACT,
    FRESNO_IMPACT,
    FLAUTA_HEAL,
    BE_ATTACKED,
    BE_HEALED,
};

struct ActiveFx {
    uint32_t targetId = 0;
    uint32_t startMs = 0;
    int fixedPixelX = 0;
    int fixedPixelY = 0;
    FxType type = FxType::DEFAULT;
};

#endif
