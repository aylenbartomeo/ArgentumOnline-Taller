#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "CityPrefab.h"

namespace {
constexpr int W = 100;
constexpr int H = 100;
constexpr int GRASS = 108;
constexpr int WATER = 109;
constexpr int STONE = 17;

constexpr int TREE = 11;
constexpr int PALM = 42;

struct Builder {
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> ground2;
    std::vector<std::vector<int>> decoration;
    std::vector<std::vector<int>> roofs;
    std::vector<std::vector<int>> indoor;
    nlohmann::json obstacles = nlohmann::json::array();
    nlohmann::json npcs = nlohmann::json::array();
    nlohmann::json monsters = nlohmann::json::array();
    nlohmann::json safeZones = nlohmann::json::array();

    Builder():
            ground(H, std::vector<int>(W, GRASS)),
            ground2(H, std::vector<int>(W, 0)),
            decoration(H, std::vector<int>(W, 0)),
            roofs(H, std::vector<int>(W, 0)),
            indoor(H, std::vector<int>(W, 0)) {}

    bool inside(int x, int y) const { return x >= 0 && x < W && y >= 0 && y < H; }

    void block(int x, int y) {
        if (inside(x, y)) {
            obstacles.push_back({{"x", x}, {"y", y}});
        }
    }

    void deco(int x, int y, int tile) {
        if (inside(x, y)) {
            decoration[y][x] = tile;
        }
    }

    void floorRect(int x0, int y0, int x1, int y1, int tile) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                if (inside(x, y)) {
                    ground[y][x] = tile;
                }
            }
        }
    }

    void path(int x0, int y0, int x1, int y1) { floorRect(x0, y0, x1, y1, STONE); }

    void foam(int x, int y, int id) {
        if (inside(x, y)) {
            ground2[y][x] = id + 1;
        }
    }

    void foamRing(int x0, int y0, int x1, int y1) {
        for (int x = x0; x <= x1; x += 2) {
            foam(x, y0 - 1, 210);
            foam(x, y1, 211);
        }
        for (int y = y0; y <= y1; y += 2) {
            foam(x0 - 1, y, 213);
            foam(x1, y, 212);
        }
        foam(x0 - 1, y0 - 1, 214);
        foam(x1, y0 - 1, 215);
        foam(x0 - 1, y1, 216);
        foam(x1, y1, 217);
    }

    void water(int x0, int y0, int x1, int y1) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                if (inside(x, y)) {
                    ground[y][x] = WATER;
                    block(x, y);
                }
            }
        }
        foamRing(x0, y0, x1, y1);
    }

    void tree(int x, int y, int tile) {
        deco(x, y, tile);
        if (tile == PALM) {
            block(x + 2, y);
        } else {
            block(x + 3, y);
            block(x + 4, y);
        }
    }

    void monster(int x, int y, const std::string& type) {
        monsters.push_back({{"type", type}, {"x", x}, {"y", y}});
    }

    void town(int ox, int oy, const std::string& name) {
        const CityPrefab& prefab = getCityPrefab();
        for (const CityCell& c: prefab.ground) {
            if (inside(ox + c.dx, oy + c.dy)) {
                ground[oy + c.dy][ox + c.dx] = c.value;
            }
        }
        for (const CityCell& c: prefab.decoration) {
            deco(ox + c.dx, oy + c.dy, c.value);
        }
        for (const CityCell& c: prefab.roofs) {
            if (inside(ox + c.dx, oy + c.dy)) {
                roofs[oy + c.dy][ox + c.dx] = c.value;
            }
        }
        for (const CityCell& c: prefab.indoor) {
            if (inside(ox + c.dx, oy + c.dy)) {
                indoor[oy + c.dy][ox + c.dx] = c.value;
            }
        }
        for (const CityCell& c: prefab.obstacles) {
            block(ox + c.dx, oy + c.dy);
        }
        std::transform(
                prefab.npcs.begin(), prefab.npcs.end(), std::back_inserter(npcs),
                [ox, oy](const CityNpc& n) {
                    return nlohmann::json{{"type", n.type}, {"x", ox + n.dx}, {"y", oy + n.dy}};
                });
        safeZones.push_back({{"name", name},
                             {"x", ox + prefab.safeDx},
                             {"y", oy + prefab.safeDy},
                             {"width", prefab.safeW},
                             {"height", prefab.safeH}});
    }
};
}  // namespace

int main() {
    Builder b;

    b.town(3, 3, "Nix");
    b.town(53, 3, "Ullathorpe");
    b.town(28, 63, "Banderbill");

    b.water(38, 42, 62, 56);

    b.path(24, 36, 25, 86);
    b.path(46, 17, 53, 18);

    const int palms[][2] = {{42, 40}, {54, 40}, {32, 46}, {32, 53},
                            {64, 45}, {64, 53}, {44, 65}, {56, 65}};
    for (const int* p: palms) {
        b.tree(p[0], p[1], PALM);
    }
    const int forest[][2] = {{8, 40},  {12, 52}, {18, 45}, {27, 59}, {33, 40}, {48, 38},
                             {52, 68}, {58, 40}, {70, 58}, {76, 42}, {82, 52}, {88, 46},
                             {92, 58}, {8, 70},  {14, 82}, {10, 92}, {20, 75}, {76, 70},
                             {82, 85}, {88, 72}, {92, 90}, {74, 92}};
    for (const int* t: forest) {
        b.tree(t[0], t[1], TREE);
    }

    b.monster(34, 40, "goblin");
    b.monster(66, 46, "orc");
    b.monster(16, 58, "zombie");
    b.monster(86, 48, "goblin");
    b.monster(20, 68, "orc");
    b.monster(84, 82, "zombie");

    nlohmann::json m;
    m["width"] = W;
    m["height"] = H;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 20}, {"y", 30}};
    m["ground"] = b.ground;
    m["ground2"] = b.ground2;
    m["decoration"] = b.decoration;
    m["roofs"] = b.roofs;
    m["indoor"] = b.indoor;
    m["obstacles"] = b.obstacles;
    m["npcs"] = b.npcs;
    m["monsters"] = b.monsters;
    m["safeZones"] = b.safeZones;

    std::ofstream out("maps/defaultMap.json");
    out << m.dump(4);
    return 0;
}
