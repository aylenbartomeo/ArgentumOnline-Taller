#include "IndoorRegions.h"

#include <utility>
#include <vector>

IndoorRegions::IndoorRegions(const std::vector<std::vector<int>>& indoor) {
    height = static_cast<int>(indoor.size());
    width = height > 0 ? static_cast<int>(indoor[0].size()) : 0;
    region.assign(height, std::vector<int>(width, -1));

    int next = 0;
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            if (indoor[r][c] == 0 || region[r][c] != -1) {
                continue;
            }
            std::vector<std::pair<int, int>> stack = {{c, r}};
            region[r][c] = next;
            while (!stack.empty()) {
                std::pair<int, int> cell = stack.back();
                stack.pop_back();
                int cc = cell.first;
                int rr = cell.second;
                const int dx[4] = {1, -1, 0, 0};
                const int dy[4] = {0, 0, 1, -1};
                for (int k = 0; k < 4; ++k) {
                    int nc = cc + dx[k];
                    int nr = rr + dy[k];
                    if (nc < 0 || nc >= width || nr < 0 || nr >= height) {
                        continue;
                    }
                    if (indoor[nr][nc] != 0 && region[nr][nc] == -1) {
                        region[nr][nc] = next;
                        stack.push_back({nc, nr});
                    }
                }
            }
            ++next;
        }
    }
}

int IndoorRegions::regionAt(int col, int row) const {
    if (col < 0 || col >= width || row < 0 || row >= height) {
        return -1;
    }
    return region[row][col];
}

bool IndoorRegions::roofHidden(int roofCol, int roofRow, int playerCol, int playerRow) const {
    int playerRegion = regionAt(playerCol, playerRow);
    if (playerRegion < 0) {
        return false;
    }
    return regionAt(roofCol, roofRow) == playerRegion;
}
