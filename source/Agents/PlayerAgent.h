/**
 * @file PlayerAgent.h
 * @author Logan Rimarcik
 *
 * TODO: Unsure who is responsible for this file. Temporary placeholder for now
 */

#ifndef CSE498COMPA_PLAYERAGENT_H
#define CSE498COMPA_PLAYERAGENT_H
#include "core/AgentBase.hpp"

namespace cse498
{

class PlayerAgent : public AgentBase
{
private:

public:
    PlayerAgent(const std::string & name, const WorldBase & world) : AgentBase(0, name, world) {}

    // TODO: fill out the function -- 8 directional movements.
    [[nodiscard]] size_t SelectAction([[maybe_unused]] const WorldGrid &grid) override { return 0; }
};

}



#endif //CSE498COMPA_PLAYERAGENT_H