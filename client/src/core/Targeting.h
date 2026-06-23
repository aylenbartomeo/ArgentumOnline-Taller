#ifndef TARGETING_H
#define TARGETING_H

#include <cstdint>
#include <optional>

#include "common/include/dto/Snapshot.h"

struct Cell {
    int col;
    int row;
};

Cell screenToCell(int screenX, int screenY, int cameraX, int cameraY, int tileSize);

std::optional<uint32_t> pickTargetAt(int col, int row, const SnapshotDTO& snap, uint32_t selfId,
                                     int maxRange);

const EntityDTO* findEntityById(const SnapshotDTO& snap, uint32_t id);

// Busca un NPC en la posición indicada y verifica que se encuentre dentro del rango máximo
// permitido respecto al jugador.
std::optional<uint32_t> pickNpcTargetAt(int col, int row, const SnapshotDTO& snap, uint32_t selfId,
                                        int maxRange);

#endif
