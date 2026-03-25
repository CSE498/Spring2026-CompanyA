/**
 * @brief Action IDs for DemoSimpleWorldG2 — must stay in sync with ConfigAgent / DoAction.
 */
#pragma once

#include <cstddef>

namespace cse498::DemoSimpleWorldG2Actions {

inline constexpr std::size_t REMAIN_STILL = 0;
inline constexpr std::size_t MOVE_UP = 1;
inline constexpr std::size_t MOVE_DOWN = 2;
inline constexpr std::size_t MOVE_LEFT = 3;
inline constexpr std::size_t MOVE_RIGHT = 4;
inline constexpr std::size_t INTERACT = 5;
inline constexpr std::size_t QUIT = 6;

} // namespace cse498::DemoSimpleWorldG2Actions
