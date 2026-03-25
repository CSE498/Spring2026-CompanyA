/**
 * @file TestPlayer.cpp
 * @author lrima
 */


#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "Agents/PlayerAgent.hpp"
#include "core/WorldBase.hpp"

namespace cse498
{
    class MockWorldBase : public WorldBase
    {
    public:
        MockWorldBase() : WorldBase() {}
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