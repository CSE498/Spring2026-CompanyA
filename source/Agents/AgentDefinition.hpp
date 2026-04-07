/**
 * @file AgentDefinition.hpp
 * @author Logan - G2
 *
 * Information needed to define an agent
 *
 * This is defined separately since it may be used a lot
 */

#pragma once
#include <string>
#include "core/WorldPosition.hpp"

namespace cse498
{
/**
 * Defines what an agent needs to be constructed
 */
struct AgentDefinition
{
    /// Name of agent
    std::string mName;
    /// Level of agent
    size_t mLevel;
    /// Position of agent
    WorldPosition mSpawn;

    AgentDefinition(std::string name, const size_t level, const WorldPosition &spawn) : mName(std::move(name)),
        mLevel(level), mSpawn(spawn)
    {
    }
};
}
