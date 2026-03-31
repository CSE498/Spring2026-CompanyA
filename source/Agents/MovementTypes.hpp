/**
 * @file MovementTypes.hpp
 * @author Logan Rimarcik
 *
 * Defines some movement maps or other movement information
 */

#pragma once

#include <cstddef>

namespace cse498
{

    class MovementTypes
    {
    private:

    public:
        [[nodiscard]] static size_t DeltaToMoveAction(double dx, double dy);

    };

}





