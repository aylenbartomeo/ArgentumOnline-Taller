#include "Map.h"
#include <cmath>

Map::Map(const std::string& toml_filepath) {}

// Inicializamos el mapa con dimensiones por defecto
Map::Map() : width(100), height(100) {
    // Inicializamos la grilla de colisiones de 100x100 en false (libre)
    collision_grid.resize(width, std::vector<bool>(height, false));
    
    // TODO: Cuando tus compañeros hagan la carga de TOML, 
    // poblarán 'mapElements' y llamarán a 'generate_collision_grid()'
}

int Map::heightLimit() const {
    return this->height;
}

int Map::widthLimit() const {
    return this->width;
}

std::pair<float, float> Map::getInitialPosition() {
    return {50.0f, 50.0f};
}

Area Map::initArea(const int x, const int y, const int width, const int height) {
    Area area;
    area.x = x;
    area.y = this->height - y - height;
    area.width = width;
    area.height = height;
    return area;
}

void Map::load_from_toml(const std::string& filepath) {}

void Map::generate_collision_grid() {
    // Reseteamos dimensiones reales si vinieran del archivo
    collision_grid.assign(width, std::vector<bool>(height, false));

    // Marcamos en la matriz qué celdas específicas están ocupadas
    for (const auto& element : mapElements) {
        if (element.type == MapElementType::OBSTACLE) {
            // Si el obstáculo abarca varias celdas, las marcamos todas
            for (int x = element.area.x; x < element.area.x + element.area.width; ++x) {
                for (int y = element.area.y; y < element.area.y + element.area.height; ++y) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        collision_grid[x][y] = true;
                    }
                }
            }
        }
    }
}

bool Map::attackColision(float pos_x, float pos_y) const{
    float maxPosXPlayer = pos_x + 20; 
    float minPosXPlayer = pos_x - 20;
    float maxPosYPlayer = pos_y + 20; 
    float minPosYPlayer = pos_y - 20;
    for (const MapElement &element : this->mapElements) {
        if (element.type == OBSTACLE) {
            float maxPosXObstaculo = element.area.x + element.area.width;
            float minPosXObstaculo = element.area.x;
            float maxPosYObstaculo = element.area.y + element.area.height;
            float minPosYObstaculo = element.area.y;
            if (maxPosXPlayer > minPosXObstaculo && minPosXPlayer < maxPosXObstaculo &&
                maxPosYPlayer > minPosYObstaculo && minPosYPlayer < maxPosYObstaculo) {
                return true; 
            }
        }
    }
    return false;
}

bool Map::playerColision(float pos_x, float pos_y) const {
    // Convertimos la posición flotante del jugador a índices enteros de la grilla
    int grid_x = static_cast<int>(std::floor(pos_x));
    int grid_y = static_cast<int>(std::floor(pos_y));

    // Si intenta salirse de los límites del mapa, lo tratamos como colisión (pared invisible)
    if (grid_x < 0 || grid_x >= width || grid_y < 0 || grid_y >= height) {
        return true;
    }

    // Retorna true inmediatamente si la celda está bloqueada. ¡Sin bucles for!
    return collision_grid[grid_x][grid_y];
}

bool Map::isCitizenArea(float pos_x, float pos_y) const {
    float maxPosXCitizen = citizenArea.area.x + citizenArea.area.width;
    float minPosXCitizen = citizenArea.area.x;
    float maxPosYCitizen = citizenArea.area.y + citizenArea.area.height;
    float minPosYCitizen = citizenArea.area.y;

    return (pos_x >= minPosXCitizen && pos_x <= maxPosXCitizen &&
            pos_y >= minPosYCitizen && pos_y <= maxPosYCitizen);
}

const std::vector<MapElement>& Map::getElements() const {
    return this->mapElements;
}
