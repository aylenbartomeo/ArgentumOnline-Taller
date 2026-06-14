#ifndef INDOOR_REGIONS_H
#define INDOOR_REGIONS_H

#include <vector>

class IndoorRegions {
private:
    std::vector<std::vector<int>> region;
    int width;
    int height;

public:
    explicit IndoorRegions(const std::vector<std::vector<int>>& indoor);

    int regionAt(int col, int row) const;
    bool roofHidden(int roofCol, int roofRow, int playerCol, int playerRow) const;
};

#endif
