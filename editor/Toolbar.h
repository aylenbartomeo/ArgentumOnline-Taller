#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <vector>

enum class Tool { OVERLAY, MONSTER, CITIZEN, CITY, ERASER, SPAWN };

enum class ToolbarAction { NONE, TOOL_CHANGED, SAVE };

class Toolbar {
public:
    struct Button {
        int x;
        int y;
        int w;
        int h;
        ToolbarAction action;
        Tool tool;
    };

private:
    std::vector<Button> buttons;
    Tool activeTool;

public:
    Toolbar();

    void addToolButton(int x, int y, int w, int h, Tool tool);
    void addActionButton(int x, int y, int w, int h, ToolbarAction action);

    ToolbarAction handleClick(int x, int y);

    Tool getActiveTool() const;
    const std::vector<Button>& getButtons() const;
};

#endif
