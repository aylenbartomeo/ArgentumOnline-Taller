#ifndef CHAR_RECT_H
#define CHAR_RECT_H

struct CharRect {
    int x;
    int y;
    int w;
    int h;
};

inline CharRect getCharRect(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    int col = uc % 16;
    int row = uc / 16;
    return {col * 128, row * 128, 128, 128};
}

#endif
