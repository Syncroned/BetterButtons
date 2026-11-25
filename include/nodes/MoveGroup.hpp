#pragma once

#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/utils/ZStringView.hpp>

namespace bem::ui {

class MoveGroup : public cocos2d::CCMenu {
public:
    static MoveGroup* create(
        geode::ZStringView label,
        CCMenuItemSpriteExtra* up,
        CCMenuItemSpriteExtra* down,
        CCMenuItemSpriteExtra* left,
        CCMenuItemSpriteExtra* right
    );
protected:
    bool init(
        geode::ZStringView label,
        CCMenuItemSpriteExtra* up,
        CCMenuItemSpriteExtra* down,
        CCMenuItemSpriteExtra* left,
        CCMenuItemSpriteExtra* right
    );
};

}
