#include <iostream>

#include "ScreenEditor.h"

int main() {
    try {
        ScreenEditor editor;
        editor.run();
    } catch (const std::exception& e) {
        std::cerr << "Error en el editor: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
