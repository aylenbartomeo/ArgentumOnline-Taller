#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace {
constexpr int W = 80;
constexpr int H = 80;
constexpr int GRASS = 108;
constexpr int WATER = 109;
constexpr int STONE = 17;

constexpr int TREE = 11;
constexpr int PALM = 42;
constexpr int HOUSE_STONE = 134;
constexpr int HOUSE_WOOD = 131;
constexpr int ALTAR = 155;
constexpr int SIGN = 1;

struct Builder {
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> ground2;
    std::vector<std::vector<int>> decoration;
    nlohmann::json obstacles = nlohmann::json::array();
    nlohmann::json npcs = nlohmann::json::array();
    nlohmann::json monsters = nlohmann::json::array();
    nlohmann::json safeZones = nlohmann::json::array();

    Builder():
            ground(H, std::vector<int>(W, GRASS)),
            ground2(H, std::vector<int>(W, 0)),
            decoration(H, std::vector<int>(W, 0)) {}

    void foam(int x, int y, int id) {
        if (inside(x, y)) {
            ground2[y][x] = id + 1;
        }
    }

    void foamRing(int x0, int y0, int x1, int y1) {
        for (int x = x0; x <= x1; x += 2) {
            foam(x, y0 - 1, 89);
            foam(x, y1 + 1, 87);
        }
        for (int y = y0; y <= y1; y += 2) {
            foam(x0 - 1, y, 90);
            foam(x1 + 1, y, 88);
        }
        foam(x0 - 1, y0 - 1, 96);
        foam(x1 + 1, y0 - 1, 94);
        foam(x0 - 1, y1 + 1, 95);
        foam(x1 + 1, y1 + 1, 93);
    }

    bool inside(int x, int y) const { return x >= 0 && x < W && y >= 0 && y < H; }

    void block(int x, int y) {
        if (inside(x, y)) {
            obstacles.push_back({{"x", x}, {"y", y}});
        }
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

    void path(int x0, int y0, int x1, int y1) {
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                if (inside(x, y)) {
                    ground[y][x] = STONE;
                }
            }
        }
    }

    void deco(int x, int y, int tile) {
        if (inside(x, y)) {
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

    void town(int ox, int oy, const std::string& name) {
        path(ox + 1, oy + 10, ox + 28, oy + 16);
        path(ox + 14, oy + 16, ox + 15, oy + 19);
        house(ox + 1, oy + 7, HOUSE_STONE, "merchant");
        house(ox + 11, oy + 7, HOUSE_WOOD, "banker");
        house(ox + 21, oy + 7, HOUSE_STONE, "priest");
        deco(ox + 23, oy + 13, ALTAR);
        deco(ox + 9, oy + 17, SIGN);
        safeZones.push_back(
                {{"name", name}, {"x", ox}, {"y", oy}, {"width", 30}, {"height", 20}});
    }
};
}  // namespace

int main() {
    Builder b;

    b.town(4, 50, "Nix");
    b.town(44, 4, "Ullathorpe");
    b.town(44, 52, "Banderbill");

    b.water(8, 8, 28, 24);

    const int palms[][2] = {{4, 10}, {30, 12}, {7, 27}, {29, 22}, {16, 4}, {2, 20}};
    for (const auto& p : palms) {
        b.tree(p[0], p[1], PALM);
    }
    const int forest[][2] = {{35, 30}, {38, 40}, {34, 48}, {40, 15}, {36, 64},
                             {8, 38},  {12, 44}, {38, 5},  {42, 35}, {30, 30}};
    for (const auto& t : forest) {
        b.tree(t[0], t[1], TREE);
    }

    b.monster(35, 35, "goblin");
    b.monster(60, 38, "orc");
    b.monster(16, 36, "zombie");
    b.monster(38, 74, "goblin");

    nlohmann::json m;
    m["width"] = W;
    m["height"] = H;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 18}, {"y", 62}};
    m["ground"] = b.ground;
    m["ground2"] = b.ground2;
    m["decoration"] = b.decoration;
    m["roofs"] = std::vector<std::vector<int>>(H, std::vector<int>(W, 0));
    m["indoor"] = std::vector<std::vector<int>>(H, std::vector<int>(W, 0));
    m["obstacles"] = b.obstacles;
    m["npcs"] = b.npcs;
    m["monsters"] = b.monsters;
    m["safeZones"] = b.safeZones;

    std::ofstream out("maps/defaultMap.json");
    out << m.dump(4);
    return 0;
}
