#include "nodes/GroupedEditMenu.hpp"
#include "utils/EditMenu.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Layout.hpp>

using namespace geode::prelude;

namespace bem::ui {

static constexpr size_t GROUPS_PER_PAGE = 4;

GroupedEditMenu* GroupedEditMenu::create(EditorUI* editor) {
    auto ret = new GroupedEditMenu();
    if (ret->init(editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GroupedEditMenu::init(EditorUI* editor) {
    if (!CCNode::init()) return false;

    m_editor = editor;

    auto winSize = CCDirector::get()->getWinSize();
    // 6 units short of the toolbar so the bottom row clears the edge
    setContentSize({winSize.width / 2.f - 5.f, CCDirector::get()->getScreenBottom() + editor->m_toolbarHeight - 6.f});
    setAnchorPoint({0.5f, 0.f});

    m_groupRow = CCNode::create();
    m_groupRow->setLayout(RowLayout::create()
        ->setGap(3.f)
    );
    m_groupRow->setContentWidth(getContentWidth() - 40.f);
    m_groupRow->setAnchorPoint({0.5f, 0.5f});
    m_groupRow->setID("group-row"_spr);
    addChildAtPosition(m_groupRow, Anchor::Center, {0.f, 10.f});

    auto pageMenu = CCMenu::create();
    pageMenu->ignoreAnchorPointForPosition(false);
    pageMenu->setAnchorPoint({0.5f, 0.5f});
    pageMenu->setContentSize(getContentSize());
    pageMenu->setID("page-menu"_spr);

    auto prevBtn = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_02_001.png", 0.5f, [this] (auto sender) {
        onMovePage(-1);
    });
    prevBtn->setID("prev-page-button"_spr);
    pageMenu->addChildAtPosition(prevBtn, Anchor::Left, {10.f, 0.f});

    auto nextBtn = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_02_001.png", 0.5f, [this] (auto sender) {
        onMovePage(1);
    });
    static_cast<CCSprite*>(nextBtn->getNormalImage())->setFlipX(true);
    nextBtn->setID("next-page-button"_spr);
    pageMenu->addChildAtPosition(nextBtn, Anchor::Right, {-10.f, 0.f});

    addChildAtPosition(pageMenu, Anchor::Center, {0.f, 10.f});

    m_bottomRow = CCMenu::create();
    m_bottomRow->setLayout(RowLayout::create()
        ->setDefaultScaleLimits(0.1f, 0.5f)
    );
    m_bottomRow->setContentWidth(getContentWidth());
    m_bottomRow->setID("bottom-row"_spr);
    addChildAtPosition(m_bottomRow, Anchor::Bottom, {0.f, 15.f});

    updateMenu();
    schedule(schedule_selector(GroupedEditMenu::enforceEditBarHidden));

    return true;
}

// the EditorTab API re-shows the edit tab bar on its own timing (mode
// switches, ui updates) and tinker's scrollbar lives inside it, so hiding it
// once isn't enough
void GroupedEditMenu::enforceEditBarHidden(float dt) {
    if (!isVisible() || !utils::edit_menu::isLegacy(m_editor)) return;
    if (!m_editor->m_editButtonBar->isVisible()) return;

    m_editor->m_editButtonBar->setVisible(false);
}

void GroupedEditMenu::onMovePage(int direction) {
    if (m_groups.empty()) return;

    auto lastPage = (m_groups.size() - 1) / GROUPS_PER_PAGE;
    if (direction > 0) {
        m_page = m_page < lastPage ? m_page + 1 : 0;
    }
    else {
        m_page = m_page > 0 ? m_page - 1 : lastPage;
    }

    updatePage();
}

void GroupedEditMenu::updatePage() {
    if (m_groups.empty()) return;

    auto lastPage = (m_groups.size() - 1) / GROUPS_PER_PAGE;
    m_page = std::min(m_page, lastPage);

    auto first = m_page * GROUPS_PER_PAGE;
    for (size_t i = 0; i < m_groups.size(); i++) {
        m_groups[i]->setVisible(i >= first && i < first + GROUPS_PER_PAGE);
    }

    m_groupRow->updateLayout();
}

void GroupedEditMenu::resetGroups() {
    m_groups.clear();
    m_groupRow->removeAllChildren();
    m_bottomRow->removeAllChildren();
    m_page = 0;
}

void GroupedEditMenu::updateMenu(bool show) {
    if (!utils::edit_menu::isLegacy(m_editor)) {
        setVisible(false);
        return;
    }

    auto editBar = m_editor->m_editButtonBar;
    // the edit bar is recreated when switching between the legacy and new
    // edit menu, groups built from the old bar's buttons are stale
    if (m_sourceBar.lock().data() != editBar) {
        resetGroups();
        m_sourceBar = editBar;
    }
    editBar->setVisible(false);

    if (!show || m_editor->m_uiHidden || !utils::edit_menu::isEditTabShown()) {
        setVisible(false);
        return;
    }

    auto tab = utils::edit_menu::getEditTab();
    if (!tab) {
        setVisible(false);
        return;
    }

    // the EditorTab API keeps two arrays for the edit tab, buttons have to be
    // pulled out of both or they get re-added on the next reload
    std::vector<std::pair<Ref<CCMenuItemSpriteExtra>, CCArray*>> sources;
    auto collect = [&] (CCArray* array) {
        if (!array) return;
        for (auto btn : CCArrayExt<CCMenuItemSpriteExtra>(array)) {
            sources.emplace_back(btn, array);
        }
    };

    collect(tab->m_buttonArray);
    if (tab->getItems() != tab->m_buttonArray) {
        collect(tab->getItems());
    }

    std::vector<std::pair<std::string, std::string>> newGroups;
    std::vector<Ref<CCMenuItemSpriteExtra>> buttons;

    for (auto& [btn, array] : sources) {
        btn->removeFromParent();
        array->removeObject(btn);

        auto id = std::string(btn->getID());
        if (m_bottomRow->getChildByID(id)) continue;

        std::string modID;
        if (auto slash = id.find('/'); slash != std::string::npos) {
            modID = id.substr(0, slash + 1);
            id = id.substr(slash + 1);
        }

        if (id.starts_with("move-")) {
            // move-<direction>-<group>
            auto group = id.substr(id.find('-', 5) + 1);

            auto groupID = Mod::get()->expandSpriteName(group + "-group");
            auto existing = std::ranges::find_if(m_groups, [&] (auto& g) { return g->getID() == groupID; });
            if (existing != m_groups.end()) continue;

            auto queued = std::ranges::find_if(newGroups, [&] (auto& g) { return g.second == group; });
            if (queued == newGroups.end()) {
                newGroups.emplace_back(modID, group);
            }
        }

        buttons.push_back(btn);
    }

    auto findButton = [&] (std::string_view id) -> CCMenuItemSpriteExtra* {
        auto it = std::ranges::find_if(buttons, [&] (auto& btn) { return btn->getID() == id; });
        return it != buttons.end() ? it->data() : nullptr;
    };

    for (auto& [modID, group] : newGroups) {
        auto up = findButton(modID + "move-up-" + group);
        auto down = findButton(modID + "move-down-" + group);
        auto left = findButton(modID + "move-left-" + group);
        auto right = findButton(modID + "move-right-" + group);
        // incomplete groups fall through to the bottom row
        if (!up || !down || !left || !right) continue;

        const char* label = "";
        if (group == "half-button") label = "1/2";
        else if (group == "quarter-button") label = "1/4";
        else if (group == "eighth-button") label = "1/8";
        else if (group == "unit-button") label = "Pixel";

        auto moveGroup = MoveGroup::create(label, up, down, left, right);
        moveGroup->setID(Mod::get()->expandSpriteName(group + "-group"));
        m_groups.push_back(moveGroup);
        m_groupRow->addChild(moveGroup);
    }

    for (auto& btn : buttons) {
        if (!btn->getParent()) {
            m_bottomRow->addChild(btn);
        }
    }

    updatePage();
    m_bottomRow->updateLayout();

    setVisible(true);
    setScale(editBar->getScale());
}

}
