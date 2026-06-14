#include "CityPrefab.h"

namespace {
constexpr int STONE = 17;
constexpr int LAWN = 219;
constexpr int WOOD = 166;
constexpr int CHURCH = 201;
constexpr int BANK = 202;
constexpr int STALL = 203;
constexpr int CHURCH_ROOF = 204;
constexpr int BANK_ROOF = 205;
constexpr int STORE_MARGIN = 2;

struct Recorder {
    CityPrefab prefab;

    void floorRect(int x0, int y0, int x1, int y1, int tile) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                prefab.ground.push_back({x, y, tile});
            }
        }
    }

    void path(int x0, int y0, int x1, int y1) { floorRect(x0, y0, x1, y1, STONE); }

    void block(int x, int y) { prefab.obstacles.push_back({x, y, 1}); }

    void building(int ax, int ay, int decoVal, int roofVal, int wT, int hT, int doorHalf,
                  int backWallTiles, int floorTile, const std::string& zoneName,
                  const std::string& npcType, int npcX, int npcY) {
        prefab.decoration.push_back({ax, ay, decoVal});
        int x1 = ax + wT - 1;
        int y0 = ay - hT + 1;
        floorRect(ax, y0, x1, ay, floorTile);
        int cx = ax + wT / 2;
        for (int x = ax; x <= x1; ++x) {
            for (int wy = y0; wy < y0 + backWallTiles; ++wy) {
                block(x, wy);
            }
            bool door = (x >= cx - doorHalf && x <= cx + doorHalf);
            if (!door) {
                block(x, ay);
            }
        }
        for (int y = y0 + backWallTiles; y <= ay; ++y) {
            block(ax, y);
            block(x1, y);
        }
        prefab.roofs.push_back({ax, ay, roofVal});
        for (int y = y0; y <= ay; ++y) {
            for (int x = ax; x <= x1; ++x) {
                prefab.indoor.push_back({x, y, 1});
            }
        }
        prefab.npcs.push_back({npcType, npcX, npcY});
        prefab.buildings.push_back({zoneName, ax, y0, wT, hT});
    }

    void stall(int ax, int ay, int wT, const std::string& zoneName, const std::string& npcType,
               int npcX, int npcY) {
        prefab.decoration.push_back({ax, ay, STALL});
        for (int x = ax; x <= ax + wT - 1; ++x) {
            block(x, ay);
            block(x, ay - 1);
        }
        prefab.npcs.push_back({npcType, npcX, npcY});
        prefab.buildings.push_back({zoneName, ax - STORE_MARGIN, (ay - 1) - STORE_MARGIN,
                                    wT + 2 * STORE_MARGIN, 2 + 2 * STORE_MARGIN});
    }
};

CityPrefab buildPrefab() {
    Recorder r;
    r.prefab.width = 44;
    r.prefab.height = 34;
    r.prefab.safeDx = 1;
    r.prefab.safeDy = 5;
    r.prefab.safeW = 36;
    r.prefab.safeH = 29;
    r.floorRect(1, 5, 36, 33, LAWN);
    r.building(2, 22, CHURCH, CHURCH_ROOF, 15, 18, 1, 6, WOOD, "church", "priest", 9, 14);
    r.building(20, 18, BANK, BANK_ROOF, 15, 11, 1, 4, WOOD, "bank", "banker", 30, 13);
    r.path(9, 23, 27, 23);
    r.path(27, 18, 27, 23);
    r.path(9, 22, 9, 23);
    r.path(20, 23, 20, 31);
    r.stall(16, 30, 9, "store", "merchant", 20, 32);
    return r.prefab;
}
}  // namespace

const CityPrefab& getCityPrefab() {
    static const CityPrefab prefab = buildPrefab();
    return prefab;
}
