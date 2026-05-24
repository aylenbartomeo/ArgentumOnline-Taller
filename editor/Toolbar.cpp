#include "Toolbar.h"

Toolbar::Toolbar(): activeTool(Tool::PINCEL) {}

void Toolbar::addToolButton(int x, int y, int w, int h, Tool tool) {
    buttons.push_back({x, y, w, h, ToolbarAction::TOOL_CHANGED, tool});
}

void Toolbar::addActionButton(int x, int y, int w, int h, ToolbarAction action) {
    buttons.push_back({x, y, w, h, action, Tool::PINCEL});
}

ToolbarAction Toolbar::handleClick(int x, int y) {
    for (const auto& b: buttons) {
        if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) {
            if (b.action == ToolbarAction::TOOL_CHANGED) {
                activeTool = b.tool;
            }
            return b.action;
        }
    }
    return ToolbarAction::NONE;
}

Tool Toolbar::getActiveTool() const { return activeTool; }

const std::vector<Toolbar::Button>& Toolbar::getButtons() const { return buttons; }
