#pragma once

#include <Geode/Enums.hpp>

namespace bem::utils::edit_command {

inline constexpr auto QuarterLeft  = static_cast<EditCommand>(0x400);
inline constexpr auto QuarterRight = static_cast<EditCommand>(0x401);
inline constexpr auto QuarterUp    = static_cast<EditCommand>(0x402);
inline constexpr auto QuarterDown  = static_cast<EditCommand>(0x403);

inline constexpr auto EighthLeft   = static_cast<EditCommand>(0x404);
inline constexpr auto EighthRight  = static_cast<EditCommand>(0x405);
inline constexpr auto EighthUp     = static_cast<EditCommand>(0x406);
inline constexpr auto EighthDown   = static_cast<EditCommand>(0x407);

inline constexpr auto UnitLeft     = static_cast<EditCommand>(0x408);
inline constexpr auto UnitRight    = static_cast<EditCommand>(0x409);
inline constexpr auto UnitUp       = static_cast<EditCommand>(0x40a);
inline constexpr auto UnitDown     = static_cast<EditCommand>(0x40b);

}
