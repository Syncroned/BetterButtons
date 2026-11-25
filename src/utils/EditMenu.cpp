#include "utils/EditMenu.hpp"

using namespace geode::prelude;

namespace bem::utils::edit_menu {

bool isLegacy(EditorUI* editor) {
    return editor && editor->m_editButtonBar && !editor->m_dpadMoveNode;
}

alpha::editor_tabs::EditorTab* getEditTab() {
    auto mode = alpha::editor_tabs::getMode(alpha::editor_tabs::Edit);
    if (!mode) return nullptr;

    auto tab = mode->getTab("edit");
    if (!tab) return nullptr;

    return tab->getNode();
}

bool isEditTabShown() {
    if (alpha::editor_tabs::getCurrentModeID() != alpha::editor_tabs::Edit) return false;

    auto mode = alpha::editor_tabs::getMode(alpha::editor_tabs::Edit);
    if (!mode) return true;

    auto tab = mode->getCurrentTab();
    if (!tab) return true;

    return tab->getID() == "edit";
}

}
