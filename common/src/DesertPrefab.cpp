#include "DesertPrefab.h"

namespace {
constexpr int SIZE = 16;
constexpr int ARENA = 74;
constexpr int BORDE_ARRIBA = 145;
constexpr int BORDE_ABAJO = 144;
constexpr int BORDE_IZQ = 142;
constexpr int BORDE_DER = 143;
constexpr int ESQ_TL = 138;
constexpr int ESQ_TR = 140;
constexpr int ESQ_BL = 139;
constexpr int ESQ_BR = 141;

int tileFor(int dx, int dy) {
    bool top = dy == 0;
    bool bottom = dy == SIZE - 1;
    bool left = dx == 0;
    bool right = dx == SIZE - 1;
    if (top && left) {
        return ESQ_TL;
    }
    if (top && right) {
        return ESQ_TR;
    }
    if (bottom && left) {
        return ESQ_BL;
    }
    if (bottom && right) {
        return ESQ_BR;
    }
    if (top) {
        return BORDE_ARRIBA;
    }
    if (bottom) {
        return BORDE_ABAJO;
    }
    if (left) {
        return BORDE_IZQ;
    }
    if (right) {
        return BORDE_DER;
    }
    return ARENA;
}

DesertPrefab buildPrefab() {
    DesertPrefab p;
    p.width = SIZE;
    p.height = SIZE;
    for (int dy = 0; dy < SIZE; ++dy) {
        for (int dx = 0; dx < SIZE; ++dx) {
            p.ground.push_back({dx, dy, tileFor(dx, dy)});
        }
    }
    return p;
}
}  // namespace

const DesertPrefab& getDesertPrefab() {
    static const DesertPrefab prefab = buildPrefab();
    return prefab;
}
