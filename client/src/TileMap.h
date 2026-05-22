#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>

/**
 * Modela la grilla de terreno del mundo. Se construye a partir del texto JSON
 * del mapa (sin importar si vino de un archivo local o de la red): asi el render
 * queda desacoplado de la fuente del mapa.
 */
class TileMap {
private:
    int width;
    int height;
    int tileSize;
    int tilesetCols;
    std::string tileset;
    std::vector<std::vector<int>> tiles;

public:
    explicit TileMap(const std::string& jsonText);

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
    int getTilesetCols() const;
    const std::string& getTileset() const;

    // Indice al tileset que va en la celda (col, row).
    int tileAt(int col, int row) const;
};

#endif
