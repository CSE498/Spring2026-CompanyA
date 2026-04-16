/**
 * @brief Action IDs for DemoSimpleWorldG2 — must stay in sync with ConfigAgent / DoAction.
 */
#pragma once

#include <cstddef>
#include <string>

namespace cse498 {
    struct WorldActions // just so they aren't globals
    {
        // Note 0 should ALWAYS Be stay based on AgentBase GetActionID implementation
        static constexpr std::size_t REMAIN_STILL = 0; // "stay"
        static constexpr std::size_t MOVE_UP = 1; // "w"
        static constexpr std::size_t MOVE_DOWN = 2; // "s"
        static constexpr std::size_t MOVE_LEFT = 3; // "a"
        static constexpr std::size_t MOVE_RIGHT = 4; // "d"
        static constexpr std::size_t INTERACT = 5; // "e"
        static constexpr std::size_t QUIT = 6; // "q"

        static constexpr std::string REMAIN_STILL_STRING = "stay";
        static constexpr std::string MOVE_UP_STRING = "w";
        static constexpr std::string MOVE_DOWN_STRING = "s";
        static constexpr std::string MOVE_LEFT_STRING = "a";
        static constexpr std::string MOVE_RIGHT_STRING = "d";
        static constexpr std::string INTERACT_STRING = "e";
        static constexpr std::string QUIT_STRING = "q";
    };
} // namespace cse498
