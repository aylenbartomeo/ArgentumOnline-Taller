#ifndef MAP_H
#define MAP_H

#include <string>
#include <utility>
#include <vector>

typedef enum { BACK, OBSTACLE, FLOOR, WEAPON } MapElementType;

struct Area {
    int x;
    int y;
    int width;
    int height;
};

struct CitizenArea {
    Area area;
};

struct MapElement {
    Area area;
    MapElementType type;
};

class Map {
private:
    int width, height;
    std::vector<MapElement> mapElements;
    CitizenArea citizenArea;
    std::pair<float, float> spawn_point;
    // Una matriz de booleanos (o de una estructura Tile) donde true = bloqueado
    std::vector<std::vector<bool>> collision_grid;
    // Area initArea(const int x, const int y, const int weight, const int height);
    // void load_from_toml(const std::string& filepath);
    //  Inicializa la matriz de colisiones en base a los mapElements cargados
    void generate_collision_grid();

public:
    // Es mejor pasarle la ruta del mapa a cargar desde el inicio
    explicit Map(const std::string& toml_filepath);
    Map();  // Para pruebas con mapa vacío

    /* Devuelven las dimensiones del mapa */
    int heightLimit() const;
    int widthLimit() const;

    /* Retorna la posición inicial segura para un jugador */
    std::pair<float, float> getInitialPosition();

    /* Retorna true si la posicion es de alguno de los ciudadanos, false en caso contrario */
    bool isCitizenArea(float pos_x, float pos_y) const;

    /* Expone los elementos para que el GameLoop arme los snapshots compartidos */
    const std::vector<MapElement>& getElements() const;

    /* Retorna true si hay colisión con un ataque, false en caso contrario */
    bool attackColision(float pos_x, float pos_y) const;
    bool playerColision(float pos_x, float pos_y) const;

    /* Metodos para setear los atributos del mapa en testing */
    void setDimensions(int w, int h);
    void setCitizenArea(int x, int y, int w, int h);
    void setSpawnPoint(float x, float y);
    void setObstacleInGrid(int cell_x, int cell_y, bool is_solid);
};

#endif
