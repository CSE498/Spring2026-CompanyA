/**
 * @file TestPlayer.cpp
 * @author lrima
 */


#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Agents/Classic/PlayerAgent.hpp"
#include "../../source/core/WorldBase.hpp"

namespace cse498
{
    class MockWorldBase : public WorldBase
    {
    public:
        MockWorldBase() : WorldBase() {
            // KAREN: Create the player here to avoid interfering with other groups' demos (temp fix)
            auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
            AddAgent(std::move(p));
            mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
            assert(mPlayer);
        }
        ~MockWorldBase() override = default;
        int DoAction([[maybe_unused]] AgentBase &agent, [[maybe_unused]] size_t action_id) override { return 0; }
    };

}



using cse498::PlayerAgent;


TEST_CASE("Main Player", "[Inventory]")
{
    // Ensure that it is accessible and there is a way to access it
    cse498::MockWorldBase world;
    auto* player = world.GetPlayer(); // shoudln't ever be null
    CHECK(player);
    auto& inv = player->GetInventory();
    CHECK(inv.GetHand() == nullptr);


}