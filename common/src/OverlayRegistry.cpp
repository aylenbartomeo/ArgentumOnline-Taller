#include "OverlayRegistry.h"

const std::vector<OverlayDef>& getOverlayRegistry() {
    static const std::vector<OverlayDef> registry = {
            {"5137.png", 604, 283, 39, 35, "Caja", 0, true},
            {"5137.png", 707, 221, 25, 35, "Barril", 0, true},
            {"5137.png", 703, 358, 27, 25, "Tonel", 0, true},
            {"5137.png", 805, 230, 18, 25, "Poste", 0, true},
            {"items/espada.png", 0, 224, 32, 32, "Espada", 2000, false},
            {"items/hacha.png", 0, 224, 32, 32, "Hacha", 2001, false},
            {"items/martillo.png", 0, 224, 32, 32, "Martillo", 2002, false},
            {"items/arco-simple.png", 0, 224, 32, 32, "Arco simple", 2010, false},
            {"items/arco-compuesto.png", 0, 224, 32, 32, "Arco compuesto", 2011, false},
            {"items/vara-fresno.png", 0, 224, 32, 32, "Vara de fresno", 2020, false},
            {"items/baculo-nudoso.png", 0, 224, 32, 32, "Baculo nudoso", 2022, false},
            {"items/baculo-engarzado.png", 0, 224, 32, 32, "Baculo engarzado", 2023, false},
            {"111.png", 422, 67, 20, 26, "Flauta elfica", 2021, false},
            {"armor/armadura-cuero.png", 0, 224, 32, 32, "Armadura de cuero", 1000, false},
            {"armor/armadura-placas.png", 0, 224, 32, 32, "Armadura de placas", 1001, false},
            {"armor/tunica-azul.png", 0, 224, 32, 32, "Tunica azul", 1002, false},
            {"armor/capucha.png", 0, 0, 17, 16, "Capucha", 1010, false},
            {"armor/casco-hierro.png", 0, 0, 17, 15, "Casco de hierro", 1011, false},
            {"armor/sombrero-magico.png", 0, 0, 17, 25, "Sombrero magico", 1012, false},
            {"armor/escudo-tortuga.png", 0, 224, 32, 32, "Escudo de tortuga", 1020, false},
            {"armor/escudo-hierro.png", 0, 224, 32, 32, "Escudo de hierro", 1021, false},
            {"101.png", 458, 38, 11, 21, "Pocion de vida", 3010, false},
            {"101.png", 423, 36, 19, 23, "Pocion de mana", 3000, false},
            {"101.png", 135, 67, 17, 25, "Pocion de agilidad", 3030, false},
            {"101.png", 167, 69, 18, 22, "Pocion de fuerza", 3020, false},
            {"106.png", 0, 384, 32, 32, "Oro", 1, false, true},
            {"5137.png", 604, 283, 39, 35, "Pared", 0, true},
            {"5137.png", 604, 283, 39, 35, "Altar", 0, false},
            {"5137.png", 604, 283, 39, 35, "Mostrador", 0, false},
            {"5137.png", 604, 283, 39, 35, "Boveda", 0, false},
    };
    return registry;
}
