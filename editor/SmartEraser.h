#ifndef SMART_ERASER_H
#define SMART_ERASER_H

#include "EditorMap.h"

enum class EraseLayer { NONE, MONSTER, CITIZEN, ITEM, BLOCK };
enum class BlockKind { NONE, CITY, DUNGEON, FOREST, DESERT };

struct EraseTarget {
    EraseLayer layer = EraseLayer::NONE;
    BlockKind block = BlockKind::NONE;
};

EraseTarget topErasableAt(const EditorMap& map, int col, int row);
void smartEraseAt(EditorMap& map, int col, int row);

#endif
