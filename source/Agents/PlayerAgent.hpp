/**
 * @file PlayerAgent.h
 * @brief Human-controlled player (text input / interface agent).
 */

#ifndef CSE498COMPA_PLAYERAGENT_H
#define CSE498COMPA_PLAYERAGENT_H

#include "../core/InterfaceBase.hpp"
#include "PlayerFeatures/Inventory.hpp"

namespace cse498 {

class PlayerAgent : public InterfaceBase {
private:
    Inventory mInventory;

public:
    PlayerAgent(size_t id, const std::string &name, const WorldBase &world);

    [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;


};

}

#endif // CSE498COMPA_PLAYERAGENT_H
