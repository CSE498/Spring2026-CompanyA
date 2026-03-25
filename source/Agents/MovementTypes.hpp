/**
 * @file MovementTypes.hpp
 * @author Logan Rimarcik
 *
 * Defines some movement maps or other movement information
 */

#ifndef SPRING2026COMPANYA_MOVEMENTTYPES_H
#define SPRING2026COMPANYA_MOVEMENTTYPES_H
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





#endif //SPRING2026COMPANYA_MOVEMENTTYPES_H