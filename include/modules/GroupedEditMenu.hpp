#pragma once

#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditButtonBar.hpp>
#include "nodes/GroupedEditMenu.hpp"
#include "utils/EditMenu.hpp"

class $modify(GEMEditorUI, EditorUI) {
    static void onModify(auto& self) {
        // run after the EditorTab API so its modes and tabs exist by the time
        // our post-call code runs
        (void) self.setHookPriority("EditorUI::init", geode::Priority::VeryLatePost);
        (void) self.setHookPriority("EditorUI::showUI", geode::Priority::LatePost);
        (void) self.setHookPriority("EditorUI::updateButtons", geode::Priority::LatePost);
        (void) self.setHookPriority("EditorUI::resetUI", geode::Priority::LatePost);
    }

    bool init(LevelEditorLayer* editorLayer);
    void showUI(bool show);
    void updateButtons();
    void resetUI();

    bem::ui::GroupedEditMenu* getGroupedEditMenu();
    void updateGroupedEditMenu();
};

class $modify(GEMEditButtonBar, EditButtonBar) {
    void loadFromItems(cocos2d::CCArray* items, int r, int c, bool unkBool);
};
