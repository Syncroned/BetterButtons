#pragma once

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/utils/cocos.hpp>
#include "nodes/MoveGroup.hpp"

namespace bem::ui {

class GroupedEditMenu : public cocos2d::CCNode {
public:
    static GroupedEditMenu* create(EditorUI* editor);

    void updateMenu(bool show = true);
protected:
    bool init(EditorUI* editor);

    void onMovePage(int direction);
    void updatePage();
    void resetGroups();
    void enforceEditBarHidden(float dt);

    EditorUI* m_editor = nullptr;
    cocos2d::CCNode* m_groupRow = nullptr;
    cocos2d::CCMenu* m_bottomRow = nullptr;
    std::vector<geode::Ref<MoveGroup>> m_groups{};
    geode::WeakRef<EditButtonBar> m_sourceBar{};
    size_t m_page = 0;
};

}
