/**
 * @file MovementTypes.hpp
 * @author Logan Rimarcik
 *
 * Defines some movement maps or other movement information
 *
 * This doesn't define every possible action but is handy for getting the action ids
 * and action names of the movement actions
 *
 * TL;DR - Defines how MOVEMENTS map to IDS/Action Names
 */

#pragma once

#include <cstddef>
#include <cmath>
#include <string>

namespace cse498
{
    class PathVector;

    class MovementTypes
    {
    private:
        static constexpr double EPS = 1e-9;
        static constexpr bool Approx(double a, double b) {
            return std::abs(a - b) < EPS;
        }

    public:
        MovementTypes() = delete;
        MovementTypes(const MovementTypes&) = delete;
        MovementTypes(MovementTypes&&) = delete;
        MovementTypes& operator=(const MovementTypes&) = delete;
        MovementTypes& operator=(MovementTypes&&) = delete;


        /**
         * Gives the action id for a given movement action
         * @param dx - x part
         * @param dy - y part
         * @return action ID for the world
         */
        [[nodiscard]] static size_t GetActionID(double dx, double dy);
        /**
         * Gives the action id for a given movement action
         * @param path - path vector of both components
         * @return action ID for the world
         */
        [[nodiscard]] static size_t GetActionID(const PathVector& path);
        /**
         * Gets the action name for the given movement
         * @param dx - x component
         * @param dy - y component
         * @return name of the action "w" or "right" or "s" ..
         */
        [[nodiscard]] static std::string GetActionName(double dx, double dy);
        /**
         * Gets the action name for the given movement
         * @param path - path vector of both components
         * @return name of the action "w" or "right" or "s" ..
         */
        [[nodiscard]] static std::string GetActionName(const PathVector& path);


    };

}





