#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace {
constexpr int W = 60;
constexpr int H = 60;
constexpr int GRASS = 108;
constexpr int WATER = 109;

constexpr int TREE = 11;
constexpr int PALM = 42;
constexpr int HOUSE_STONE = 134;
constexpr int HOUSE_WOOD = 131;
constexpr int ALTAR = 155;
constexpr int SIGN = 1;

struct Builder {
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> decoration;
    nlohmann::json obstacles = nlohmann::json::array();
    nlohmann::json npcs = nlohmann::json::array();
    nlohmann::json monsters = nlohmann::json::array();

    Builder():
            ground(H, std::vector<int>(W, GRASS)), decoration(H, std::vector<int>(W, 0)) {}

    void block(int x, int y) {
        if (x >= 0 && x < W && y >= 0 && y < H) {
            obstacles.push_back({{"x", x}, {"y", y}});
        }
    }

    void water(int x0, int y0, int x1, int y1) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                if (x < 0 || x >= W || y < 0 || y >= H) {
                    continue;
                }
                ground[y][x] = WATER;
                block(x, y);
            }
        }
    }

    void deco(int x, int y, int tile) {
        if (x >= 0 && x < W && y >= 0 && y < H) {
            decoration[y][x] = tile;
        }
    }

    void tree(int x, int y, int tile) {
        deco(x, y, tile);
        block(x, y);
    }

    void house(int x, int y, int tile, const std::string& npcType) {
        deco(x, y, tile);
        for (int dy = -3; dy <= 0; ++dy) {
            for (int dx = 0; dx <= 7; ++dx) {
                block(x + dx, y + dy);
            }
        }
        npcs.push_back({{"type", npcType}, {"x", x + 3}, {"y", y + 2}});
    }

    void monster(int x, int y, const std::string& type) {
        monsters.push_back({{"type", type}, {"x", x}, {"y", y}});
    }
};
}  // namespace

int main() {
    Builder b;

    b.water(40, 6, 52, 20);
    b.water(36, 10, 39, 16);

    const int trees[][3] = {{4, 12, TREE},  {9, 8, TREE},   {6, 22, PALM},  {12, 16, TREE},
                            {48, 30, TREE}, {52, 38, PALM}, {44, 46, TREE}, {38, 52, TREE},
                            {18, 6, TREE},  {30, 10, TREE}, {54, 50, PALM}, {3, 40, TREE}};
    for (const auto& t : trees) {
        b.tree(t[0], t[1], t[2]);
    }

    b.house(12, 42, HOUSE_STONE, "merchant");
    b.house(24, 42, HOUSE_WOOD, "banker");
    b.house(17, 50, HOUSE_STONE, "priest");
    b.deco(20, 49, ALTAR);
    b.deco(9, 38, SIGN);

    b.monster(50, 52, "goblin");
    b.monster(5, 52, "orc");
    b.monster(54, 24, "zombie");
    b.monster(30, 30, "goblin");

    nlohmann::json m;
    m["width"] = W;
    m["height"] = H;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 20}, {"y", 46}};
    m["ground"] = b.ground;
    m["ground2"] = std::vector<std::vector<int>>(H, std::vector<int>(W, 0));
    m["decoration"] = b.decoration;
    m["roofs"] = std::vector<std::vector<int>>(H, std::vector<int>(W, 0));
    m["indoor"] = std::vector<std::vector<int>>(H, std::vector<int>(W, 0));
    m["obstacles"] = b.obstacles;
    m["npcs"] = b.npcs;
    m["monsters"] = b.monsters;
    m["safeZones"] = nlohmann::json::array(
            {{{"name", "Nix"}, {"x", 8}, {"y", 36}, {"width", 26}, {"height", 20}}});

    std::ofstream out("maps/defaultMap.json");
    out << m.dump(4);
    return 0;
}
