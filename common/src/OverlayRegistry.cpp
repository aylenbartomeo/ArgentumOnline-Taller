#include "OverlayRegistry.h"

const std::vector<OverlayDef>& getOverlayRegistry() {
    static const std::vector<OverlayDef> registry = {
            {"5137.png", 604, 283, 39, 35, "Caja", 0, true},
            {"5137.png", 707, 221, 25, 35, "Barril", 0, true},
            {"5137.png", 703, 358, 27, 25, "Tonel", 0, true},
            {"5137.png", 805, 230, 18, 25, "Poste", 0, true},
            {"items/espada.png", 0, 224, 32, 32, "Espada", 2000, false},
            {"items/hacha.png", 0, 224, 32, 32, "Hacha", 2001, false},
            {"items/martillo.png", 0, 192, 32, 32, "Martillo", 2002, false},
            {"101.png", 458, 38, 11, 21, "Pocion de vida", 3010, false},
            {"101.png", 423, 36, 19, 23, "Pocion de mana", 3000, false},
            {"101.png", 135, 67, 17, 25, "Pocion de agilidad", 3030, false},
            {"101.png", 167, 69, 18, 22, "Pocion de fuerza", 3020, false},
            {"101.png", 230, 65, 21, 30, "Armadura de cuero", 1000, false},
            {"101.png", 294, 129, 21, 29, "Armadura de placas", 1001, false},
            {"101.png", 71, 98, 17, 28, "Tunica azul", 1002, false},
            {"102.png", 37, 64, 22, 32, "Capucha", 1010, false},
            {"112.png", 549, 130, 23, 30, "Casco de hierro", 1011, false},
            {"101.png", 232, 41, 14, 13, "Sombrero magico", 1012, false},
            {"102.png", 288, 64, 32, 32, "Escudo de tortuga", 1020, false},
            {"112.png", 578, 128, 28, 32, "Escudo de hierro", 1021, false},
            {"106.png", 0, 384, 32, 32, "Oro", 1, false, true},
            {"5137.png", 604, 283, 39, 35, "Pared", 0, true},
            {"5137.png", 604, 283, 39, 35, "Altar", 0, false},
            {"5137.png", 604, 283, 39, 35, "Mostrador", 0, false},
            {"5137.png", 604, 283, 39, 35, "Boveda", 0, false},
    };
    return registry;
}
