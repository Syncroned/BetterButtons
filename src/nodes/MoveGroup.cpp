#include "nodes/MoveGroup.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

namespace bem::ui {

MoveGroup* MoveGroup::create(
    ZStringView label,
    CCMenuItemSpriteExtra* up,
    CCMenuItemSpriteExtra* down,
    CCMenuItemSpriteExtra* left,
    CCMenuItemSpriteExtra* right
) {
    auto ret = new MoveGroup();
    if (ret->init(label, up, down, left, right)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoveGroup::init(
    ZStringView label,
    CCMenuItemSpriteExtra* up,
    CCMenuItemSpriteExtra* down,
    CCMenuItemSpriteExtra* left,
    CCMenuItemSpriteExtra* right
) {
    if (!CCMenu::init()) return false;

    ignoreAnchorPointForPosition(false);
    setAnchorPoint({0.5f, 0.5f});
    setContentSize({60.f, 30.f});

    for (auto btn : {up, down, left, right}) {
        btn->setScale(0.5f);
        btn->m_baseScale = 0.5f;
    }

    addChildAtPosition(up, Anchor::Top, {0.f, -5.f});
    addChildAtPosition(down, Anchor::Bottom, {0.f, 5.f});
    addChildAtPosition(left, Anchor::Left, {10.f, 0.f});
    addChildAtPosition(right, Anchor::Right, {-10.f, 0.f});

    if (!label.empty()) {
        auto labelNode = CCLabelBMFont::create(label.c_str(), "bigFont.fnt");
        labelNode->setScale(0.25f);
        labelNode->setID("label"_spr);
        addChildAtPosition(labelNode, Anchor::Top, {0.f, 10.f});
    }

    return true;
}

}
