#include "Palette.h"

Palette::Palette(int panelX, int panelY, int tileDrawSize, int cols, int tileCount):
        panelX(panelX),
        panelY(panelY),
        tileDrawSize(tileDrawSize),
        cols(cols),
        tileCount(tileCount),
        selectedTile(0),
        scrollRow(0) {}

void Palette::setSelectedTile(int index) {
    if (index >= 0 && index < tileCount) {
        selectedTile = index;
    }
}

void Palette::scroll(int deltaRows) {
    int totalRows = (tileCount + cols - 1) / cols;
    scrollRow += deltaRows;
    if (scrollRow < 0) {
        scrollRow = 0;
    }
    if (scrollRow > totalRows - 1) {
        scrollRow = totalRows - 1;
    }
}

int Palette::getSelectedTile() const { return selectedTile; }
int Palette::getPanelX() const { return panelX; }
int Palette::getPanelY() const { return panelY; }
int Palette::getTileDrawSize() const { return tileDrawSize; }
int Palette::getCols() const { return cols; }
int Palette::getTileCount() const { return tileCount; }
int Palette::getScrollRow() const { return scrollRow; }
