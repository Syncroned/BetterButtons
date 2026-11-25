#include "modules/GroupedEditMenu.hpp"
#include "utils/EditMenu.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>

using namespace geode::prelude;

bool GEMEditorUI::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;
    if (!Mod::get()->getSettingValue<bool>("new-edit-menu")) return true;

    auto menu = bem::ui::GroupedEditMenu::create(this);
    menu->setID("grouped-edit-menu"_spr);
    menu->setPosition({CCDirector::get()->getWinSize().width / 2.f, 5.f});
    addChild(menu, 10);

    // late so the EditorTab API has finished moving its items around first
    addEventListener("setup-move-menu"_spr, alpha::editor_tabs::SetupMoveMenuEvent(), [this] (alpha::editor_tabs::Tab* tab) {
        updateGroupedEditMenu();
    }, Priority::Late);

    addEventListener("tab-loaded"_spr, alpha::editor_tabs::Tab::TabLoadedEvent(), [this] (alpha::editor_tabs::Tab* tab, alpha::editor_tabs::EditorTab* node) {
        if (node != bem::utils::edit_menu::getEditTab()) return;
        updateGroupedEditMenu();
    }, Priority::Late);

    addEventListener("switch-tab"_spr, alpha::editor_tabs::Tab::SwitchTabEvent(), [this] (alpha::editor_tabs::Tab* tab, bool show) {
        updateGroupedEditMenu();
    }, Priority::Late);

    addEventListener("switch-mode"_spr, alpha::editor_tabs::Mode::SwitchModeEvent(), [this] (alpha::editor_tabs::Mode* mode, bool show) {
        updateGroupedEditMenu();
    }, Priority::Late);

    return true;
}

bem::ui::GroupedEditMenu* GEMEditorUI::getGroupedEditMenu() {
    return static_cast<bem::ui::GroupedEditMenu*>(getChildByID("grouped-edit-menu"_spr));
}

void GEMEditorUI::updateGroupedEditMenu() {
    if (auto menu = getGroupedEditMenu()) {
        menu->updateMenu();
    }
}

void GEMEditorUI::showUI(bool show) {
    if (m_editorLayer && m_editorLayer->m_playbackMode == PlaybackMode::Playing) {
        show = !GameManager::get()->getGameVariable(GameVar::PlaytestNoUI);
    }

    EditorUI::showUI(show);

    if (auto menu = getGroupedEditMenu()) {
        menu->updateMenu(show);
    }
}

// the EditorTab API may re-show the vanilla edit bar in both of these
void GEMEditorUI::updateButtons() {
    EditorUI::updateButtons();
    updateGroupedEditMenu();
}

void GEMEditorUI::resetUI() {
    EditorUI::resetUI();
    updateGroupedEditMenu();
}

void GEMEditButtonBar::loadFromItems(CCArray* items, int r, int c, bool unkBool) {
    EditButtonBar::loadFromItems(items, r, c, unkBool);
    if (getID() != "edit-tab-bar") return;

    if (auto editor = static_cast<GEMEditorUI*>(EditorUI::get())) {
        editor->updateGroupedEditMenu();
    }
}
