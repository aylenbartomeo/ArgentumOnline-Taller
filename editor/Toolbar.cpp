#include "Toolbar.h"

#include <algorithm>

Toolbar::Toolbar(): activeTool(Tool::OVERLAY) {}

void Toolbar::addToolButton(int x, int y, int w, int h, Tool tool) {
    buttons.push_back({x, y, w, h, ToolbarAction::TOOL_CHANGED, tool});
}

void Toolbar::addActionButton(int x, int y, int w, int h, ToolbarAction action) {
    buttons.push_back({x, y, w, h, action, Tool::OVERLAY});
}

ToolbarAction Toolbar::handleClick(int x, int y) {
    auto it = std::find_if(buttons.begin(), buttons.end(), [x, y](const Button& b) {
        return x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h;
    });

    if (it != buttons.end()) {
        if (it->action == ToolbarAction::TOOL_CHANGED) {
            activeTool = it->tool;
        }
        return it->action;
    }
    return ToolbarAction::NONE;
}

Tool Toolbar::getActiveTool() const { return activeTool; }

const std::vector<Toolbar::Button>& Toolbar::getButtons() const { return buttons; }
