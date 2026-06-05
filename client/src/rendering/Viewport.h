#ifndef VIEWPORT_H
#define VIEWPORT_H

struct CameraOffset {
    int x;
    int y;
};

CameraOffset computeCameraOffset(int focusPxX, int focusPxY, int viewW, int viewH, int worldW,
                                 int worldH);

#endif
