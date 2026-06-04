#ifndef HEALTH_BAR_H
#define HEALTH_BAR_H

struct BarRect {
    int x;
    int y;
    int w;
    int h;
};

struct HealthBarLayout {
    bool visible;
    BarRect background;
    BarRect fill;
};

HealthBarLayout computeHealthBar(int currentHp, int maxHp, int entityX, int entityY, int tileSize);

#endif
