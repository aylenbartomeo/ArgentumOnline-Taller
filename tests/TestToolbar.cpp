#include <gtest/gtest.h>

#include "Toolbar.h"

TEST(ToolbarTest, DefaultToolIsOverlay) {
    Toolbar toolbar;
    EXPECT_EQ(toolbar.getActiveTool(), Tool::OVERLAY);
}

TEST(ToolbarTest, ClickMonsterButtonActivatesMonster) {
    Toolbar toolbar;
    toolbar.addToolButton(0, 0, 100, 30, Tool::MONSTER);
    EXPECT_EQ(toolbar.handleClick(10, 10), ToolbarAction::TOOL_CHANGED);
    EXPECT_EQ(toolbar.getActiveTool(), Tool::MONSTER);
}

TEST(ToolbarTest, ClickCitizenButtonActivatesCitizen) {
    Toolbar toolbar;
    toolbar.addToolButton(0, 0, 100, 30, Tool::CITIZEN);
    EXPECT_EQ(toolbar.handleClick(10, 10), ToolbarAction::TOOL_CHANGED);
    EXPECT_EQ(toolbar.getActiveTool(), Tool::CITIZEN);
}

TEST(ToolbarTest, ClickToolButtonChangesActiveTool) {
    Toolbar toolbar;
    toolbar.addToolButton(0, 0, 100, 30, Tool::SPAWN);
    ToolbarAction action = toolbar.handleClick(10, 10);
    EXPECT_EQ(action, ToolbarAction::TOOL_CHANGED);
    EXPECT_EQ(toolbar.getActiveTool(), Tool::SPAWN);
}

TEST(ToolbarTest, ClickEraserButtonActivatesEraser) {
    Toolbar toolbar;
    toolbar.addToolButton(0, 0, 100, 30, Tool::ERASER);
    ToolbarAction action = toolbar.handleClick(10, 10);
    EXPECT_EQ(action, ToolbarAction::TOOL_CHANGED);
    EXPECT_EQ(toolbar.getActiveTool(), Tool::ERASER);
}

TEST(ToolbarTest, ClickSaveButtonReturnsSave) {
    Toolbar toolbar;
    toolbar.addActionButton(0, 40, 100, 30, ToolbarAction::SAVE);
    EXPECT_EQ(toolbar.handleClick(10, 50), ToolbarAction::SAVE);
}

TEST(ToolbarTest, ClickEmptyAreaReturnsNone) {
    Toolbar toolbar;
    toolbar.addActionButton(0, 80, 35, 30, ToolbarAction::SAVE);
    EXPECT_EQ(toolbar.handleClick(500, 500), ToolbarAction::NONE);
}
