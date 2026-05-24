#ifndef PALETTE_H
#define PALETTE_H

class Palette {
private:
    int panelX;
    int panelY;
    int tileDrawSize;
    int cols;
    int tileCount;
    int selectedTile;

public:
    Palette(int panelX, int panelY, int tileDrawSize, int cols, int tileCount);

    void selectFromClick(int x, int y);

    int getSelectedTile() const;
    int getPanelX() const;
    int getPanelY() const;
    int getTileDrawSize() const;
    int getCols() const;
    int getTileCount() const;
};

#endif
