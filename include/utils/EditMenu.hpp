#pragma once

#include <Geode/binding/EditorUI.hpp>
// the EditorTab API's headers aren't self-contained
#include <Geode/loader/Log.hpp>
#include <Geode/utils/cocos.hpp>
#include <alphalaneous.editortab_api/API.hpp>

namespace bem::utils::edit_menu {

// the grouped menu only replaces the legacy edit button bar, the new (dpad)
// edit menu is laid out by the EditorTab API which expects its dpad buttons
// to stay where vanilla put them, so it must never be touched
bool isLegacy(EditorUI* editor);

alpha::editor_tabs::EditorTab* getEditTab();

bool isEditTabShown();

}
