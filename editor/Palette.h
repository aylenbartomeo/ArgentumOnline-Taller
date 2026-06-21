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
    int scrollRow;

public:
    Palette(int panelX, int panelY, int tileDrawSize, int cols, int tileCount);

    void setSelectedTile(int index);
    void scroll(int deltaRows);

    int getSelectedTile() const;
    int getPanelX() const;
    int getPanelY() const;
    int getTileDrawSize() const;
    int getCols() const;
    int getTileCount() const;
    int getScrollRow() const;
};

#endif
