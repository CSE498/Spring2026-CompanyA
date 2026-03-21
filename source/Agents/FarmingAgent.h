/**
* @file FarmingAgent.h
 * @brief Minimal FarmingAgent (trading NPC stub).
 */
#ifndef CSE498COMPA_FARMINGAGENT_H
#define CSE498COMPA_FARMINGAGENT_H

#include "../core/AgentBase.hpp"

namespace cse498
{

    /** Basic NPC agent used for simple trading interactions. */
    class FarmingAgent : public AgentBase
    {
    private:
        bool mAvailableForTrade = true;

    public:
        FarmingAgent(size_t id, const std::string & name, WorldBase & world);

        /** No autonomous behavior; returns 0 (no action). */
        [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override;

        /** Is trading available? */
        [[nodiscard]] bool IsAvailableForTrade() const;

        /** Set trading availability. */
        void SetAvailableForTrade(bool available);
    };

}

#endif // CSE498COMPA_FARMINGAGENT_H