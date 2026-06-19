#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "CityPrefab.h"
#include "DungeonPrefab.h"
#include "ForestPrefab.h"

namespace {
constexpr int W = 150;
constexpr int H = 150;
constexpr int GRASS = 108;

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
    nlohmann::json items = nlohmann::json::array();
    nlohmann::json dungeons = nlohmann::json::array();
    nlohmann::json forests = nlohmann::json::array();

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

    void dungeon(int ox, int oy) {
        const DungeonPrefab& prefab = getDungeonPrefab();
        for (const DungeonCell& c: prefab.ground) {
            if (inside(ox + c.dx, oy + c.dy)) {
                ground[oy + c.dy][ox + c.dx] = c.value;
            }
        }
        for (const DungeonCell& c: prefab.decoration) {
            deco(ox + c.dx, oy + c.dy, c.value);
        }
        for (const DungeonCell& c: prefab.obstacles) {
            block(ox + c.dx, oy + c.dy);
        }
        for (const DungeonItem& g: prefab.gold) {
            if (inside(ox + g.dx, oy + g.dy)) {
                items.push_back({{"id", g.itemId},
                                 {"x", ox + g.dx},
                                 {"y", oy + g.dy},
                                 {"amount", g.amount}});
            }
        }
        dungeons.push_back({{"x", ox + prefab.dungeonDx},
                            {"y", oy + prefab.dungeonDy},
                            {"width", prefab.dungeonW},
                            {"height", prefab.dungeonH}});
    }

    void forest(int ox, int oy) {
        const ForestPrefab& prefab = getForestPrefab();
        for (const ForestCell& c: prefab.decoration) {
            deco(ox + c.dx, oy + c.dy, c.value);
        }
        for (const ForestCell& c: prefab.obstacles) {
            block(ox + c.dx, oy + c.dy);
        }
        forests.push_back(
                {{"x", ox}, {"y", oy}, {"width", prefab.width}, {"height", prefab.height}});
    }
};
}  // namespace

int main() {
    Builder b;

    b.town(5, 5, "Nix");
    b.town(101, 5, "Ullathorpe");
    b.town(5, 111, "Banderbill");
    b.town(101, 111, "Lindos");

    b.dungeon(67, 8);
    b.dungeon(67, 120);
    b.dungeon(8, 60);
    b.dungeon(126, 60);
    b.dungeon(40, 60);
    b.dungeon(94, 60);

    b.forest(55, 45);
    b.forest(88, 45);
    b.forest(30, 90);
    b.forest(108, 90);

    b.monster(75, 50, "goblin");
    b.monster(75, 100, "orc");
    b.monster(30, 75, "zombie");
    b.monster(120, 75, "goblin");
    b.monster(75, 75, "orc");

    nlohmann::json m;
    m["width"] = W;
    m["height"] = H;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 75}, {"y", 95}};
    m["ground"] = b.ground;
    m["ground2"] = b.ground2;
    m["decoration"] = b.decoration;
    m["roofs"] = b.roofs;
    m["indoor"] = b.indoor;
    m["obstacles"] = b.obstacles;
    m["npcs"] = b.npcs;
    m["monsters"] = b.monsters;
    m["safeZones"] = b.safeZones;
    m["items"] = b.items;
    m["dungeons"] = b.dungeons;
    m["forests"] = b.forests;

    std::ofstream out("maps/defaultMap.json");
    out << m.dump(4);
    return 0;
}
