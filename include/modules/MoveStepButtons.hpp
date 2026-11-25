#pragma once

#include <Geode/modify/EditorUI.hpp>
#include "utils/EditMenu.hpp"

class $modify(MSBEditorUI, EditorUI) {
    static void onModify(auto& self) {
        // the EditorTab API sets up the edit tab at LatePost, this has to run after it
        (void) self.setHookPriority("EditorUI::init", geode::Priority::LatePost - 1);
    }

    struct Fields {
        geode::WeakRef<alpha::editor_tabs::EditorTab> m_populatedTab{};
    };

    bool init(LevelEditorLayer* editorLayer);
    cocos2d::CCPoint moveForCommand(EditCommand command);

    void addMoveStepButtons();
    void onMoveStep(cocos2d::CCObject* sender);
};
