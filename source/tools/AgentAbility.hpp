/**
 * @file AgentAbility.hpp
 * @author lrima
 *
 * This is a preliminary placeholder file to represent some things the agent may
 * or may not be able to do this information is used in path finding to know
 * whether the agent can walk on certain tiles or not.
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_AGENTABILITY_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_AGENTABILITY_H

namespace cse498 {
class AgentAbility {
private:
  [[maybe_unused]] bool climb = false;
  [[maybe_unused]] bool swim = false;

public:
};
} // namespace cse498

#endif // SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_AGENTABILITY_H
