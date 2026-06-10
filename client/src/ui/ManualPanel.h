#ifndef MANUAL_PANEL_H
#define MANUAL_PANEL_H

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include "../input/EventHandler.h"

class ManualPanel {
public:
    explicit ManualPanel(const std::string& fontPath);
    ~ManualPanel();

    void loadManual(const std::string& filePath);
    void update(const FrameInput& input, int windowW, int windowH);
    void render(SDL_Renderer* renderer, int windowW, int windowH);

    bool isVisible() const { return visible; }
    void toggle() { visible = !visible; }

private:
    TTF_Font* font;
    std::vector<std::string> lines;
    int scrollOffset = 0;
    bool visible = false;

    // UI dimensions
    int panelW = 600;
    int panelH = 500;
    static constexpr int PADDING = 20;

    void parseMarkdown(const std::string& content);
};

#endif
