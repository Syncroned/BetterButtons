#include "modules/MoveStepButtons.hpp"
#include "utils/EditCommandExt.hpp"
#include "utils/EditMenu.hpp"

using namespace geode::prelude;
namespace edit_command = bem::utils::edit_command;

struct MoveStepButton {
    const char* id;
    const char* frame;
    EditCommand command;
};

static constexpr std::array MOVE_STEP_BUTTONS {
    MoveStepButton{"move-up-quarter-button", "edit_upBtn5_001.png", edit_command::QuarterUp},
    MoveStepButton{"move-down-quarter-button", "edit_downBtn5_001.png", edit_command::QuarterDown},
    MoveStepButton{"move-left-quarter-button", "edit_leftBtn5_001.png", edit_command::QuarterLeft},
    MoveStepButton{"move-right-quarter-button", "edit_rightBtn5_001.png", edit_command::QuarterRight},
    MoveStepButton{"move-up-eighth-button", "edit_upBtn5_001.png", edit_command::EighthUp},
    MoveStepButton{"move-down-eighth-button", "edit_downBtn5_001.png", edit_command::EighthDown},
    MoveStepButton{"move-left-eighth-button", "edit_leftBtn5_001.png", edit_command::EighthLeft},
    MoveStepButton{"move-right-eighth-button", "edit_rightBtn5_001.png", edit_command::EighthRight},
    MoveStepButton{"move-up-unit-button", "edit_upBtn_001.png", edit_command::UnitUp},
    MoveStepButton{"move-down-unit-button", "edit_downBtn_001.png", edit_command::UnitDown},
    MoveStepButton{"move-left-unit-button", "edit_leftBtn_001.png", edit_command::UnitLeft},
    MoveStepButton{"move-right-unit-button", "edit_rightBtn_001.png", edit_command::UnitRight},
};

bool MSBEditorUI::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) return false;
    if (!Mod::get()->getSettingValue<bool>("extra-move-buttons")) return true;

    addMoveStepButtons();

    // the edit tab is rebuilt when switching between the legacy and new edit menu
    addEventListener("move-step-buttons"_spr, alpha::editor_tabs::SetupMoveMenuEvent(), [this] (alpha::editor_tabs::Tab* tab) {
        addMoveStepButtons();
    });

    return true;
}

void MSBEditorUI::addMoveStepButtons() {
    if (!bem::utils::edit_menu::isLegacy(this)) return;

    auto tab = bem::utils::edit_menu::getEditTab();
    if (!tab) return;

    // the grouped menu pulls our buttons out of the tab, so membership can't
    // tell whether a tab was already populated
    auto fields = m_fields.self();
    if (fields->m_populatedTab.lock().data() == tab) return;
    fields->m_populatedTab = tab;

    for (auto& [id, frame, command] : MOVE_STEP_BUTTONS) {
        auto btn = getSpriteButton(frame, menu_selector(MSBEditorUI::onMoveStep), nullptr, 0.9f);
        btn->setID(Mod::get()->expandSpriteName(id));
        btn->setTag(static_cast<int>(command));
        tab->getItems()->addObject(btn);
    }

    tab->reloadAllItems();
}

// calling moveObjectCall(CCObject*) directly resolves to the wrong callback
void MSBEditorUI::onMoveStep(CCObject* sender) {
    moveObjectCall(static_cast<EditCommand>(sender->getTag()));
}

CCPoint MSBEditorUI::moveForCommand(EditCommand command) {
    switch (static_cast<int>(command)) {
        case static_cast<int>(edit_command::QuarterLeft): return CCPoint{-0.25f, 0.f} * m_gridSize;
        case static_cast<int>(edit_command::QuarterRight): return CCPoint{0.25f, 0.f} * m_gridSize;
        case static_cast<int>(edit_command::QuarterUp): return CCPoint{0.f, 0.25f} * m_gridSize;
        case static_cast<int>(edit_command::QuarterDown): return CCPoint{0.f, -0.25f} * m_gridSize;

        case static_cast<int>(edit_command::EighthLeft): return CCPoint{-0.125f, 0.f} * m_gridSize;
        case static_cast<int>(edit_command::EighthRight): return CCPoint{0.125f, 0.f} * m_gridSize;
        case static_cast<int>(edit_command::EighthUp): return CCPoint{0.f, 0.125f} * m_gridSize;
        case static_cast<int>(edit_command::EighthDown): return CCPoint{0.f, -0.125f} * m_gridSize;

        case static_cast<int>(edit_command::UnitLeft): return {-0.1f, 0.f};
        case static_cast<int>(edit_command::UnitRight): return {0.1f, 0.f};
        case static_cast<int>(edit_command::UnitUp): return {0.f, 0.1f};
        case static_cast<int>(edit_command::UnitDown): return {0.f, -0.1f};

        default: return EditorUI::moveForCommand(command);
    }
}
