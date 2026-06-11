#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace {
constexpr int W = 60;
constexpr int H = 60;
constexpr int GRASS = 108;
constexpr int WATER = 109;
}  // namespace

int main() {
    std::vector<std::vector<int>> ground(H, std::vector<int>(W, GRASS));
    std::vector<std::vector<int>> empty(H, std::vector<int>(W, 0));
    nlohmann::json obstacles = nlohmann::json::array();

    for (int y = 0; y < H; ++y) {
        for (int x = 28; x <= 31; ++x) {
            ground[y][x] = WATER;
            obstacles.push_back({{"x", x}, {"y", y}});
        }
    }

    nlohmann::json m;
    m["width"] = W;
    m["height"] = H;
    m["tileSize"] = 32;
    m["tileset"] = "5108.png";
    m["tilesetCols"] = 32;
    m["spawn"] = {{"x", 6}, {"y", 9}};
    m["ground"] = ground;
    m["ground2"] = empty;
    m["decoration"] = empty;
    m["roofs"] = empty;
    m["indoor"] = empty;
    m["obstacles"] = obstacles;
    m["safeZones"] = nlohmann::json::array(
            {{{"name", "Nix"}, {"x", 2}, {"y", 2}, {"width", 10}, {"height", 10}}});
    m["npcs"] = nlohmann::json::array({
            {{"type", "merchant"}, {"x", 4}, {"y", 4}},
            {{"type", "priest"}, {"x", 6}, {"y", 4}},
            {{"type", "banker"}, {"x", 8}, {"y", 4}},
    });

    std::ofstream out("maps/defaultMap.json");
    out << m.dump(4);
    return 0;
}
