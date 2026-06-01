#include "OverlayRegistry.h"

const std::vector<OverlayDef>& getOverlayRegistry() {
    static const std::vector<OverlayDef> registry = {
            {"5137.png", 604, 283, 39, 35, "Caja"},
            {"5137.png", 707, 221, 25, 35, "Barril"},
            {"5137.png", 703, 358, 27, 25, "Tonel"},
            {"5137.png", 805, 230, 18, 25, "Poste"},
            {"5137.png", 607, 232, 34, 23, "Botella"},
            {"101.png", 225, 97, 29, 29, "Espada"},
            {"106.png", 227, 35, 27, 27, "Hacha"},
            {"101.png", 290, 197, 28, 27, "Martillo"},
            {"101.png", 165, 1, 24, 30, "Arco simple"},
            {"104.png", 485, 354, 22, 29, "Arco compuesto"},
            {"101.png", 164, 98, 26, 28, "Vara de fresno"},
            {"111.png", 422, 67, 20, 26, "Flauta elfica"},
            {"101.png", 193, 97, 30, 29, "Baculo nudoso"},
            {"101.png", 129, 129, 29, 29, "Baculo engarzado"},
            {"101.png", 458, 38, 11, 21, "Pocion de vida"},
            {"101.png", 423, 36, 19, 23, "Pocion de mana"},
            {"101.png", 135, 67, 17, 25, "Pocion de agilidad"},
            {"101.png", 167, 69, 18, 22, "Pocion de fuerza"},
            {"101.png", 230, 65, 21, 30, "Armadura de cuero"},
            {"101.png", 294, 129, 21, 29, "Armadura de placas"},
            {"101.png", 71, 98, 17, 28, "Tunica azul"},
            {"102.png", 37, 64, 22, 32, "Capucha"},
            {"112.png", 549, 130, 23, 30, "Casco de hierro"},
            {"101.png", 232, 41, 14, 13, "Sombrero magico"},
            {"102.png", 288, 64, 32, 32, "Escudo de tortuga"},
            {"112.png", 578, 128, 28, 32, "Escudo de hierro"},
    };
    return registry;
}
