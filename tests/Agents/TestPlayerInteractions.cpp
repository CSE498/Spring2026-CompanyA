/**
 * @file TestPlayerInteractions.cpp
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/Classic/Enemy.hpp"
#include "../../source/Agents/Classic/FarmingAgent.hpp"
#include "../../source/Agents/Classic/PlayerAgent.hpp"
#include "../../source/Worlds/DemoG2/DemoSimpleWorldG2.hpp"
#include "../../source/Worlds/DemoG2/WorldActions.hpp"

#include <sstream>
#include <string>

namespace cse498 {

namespace {

AgentBase* FindAgentByName(WorldBase& world, const std::string& name) {
    for (size_t i = 0; i < world.GetNumAgents(); ++i) {
        AgentBase& agent = world.GetAgentByIndex(i);
        if (agent.GetName() == name) {
            return &agent;
        }
    }
    return nullptr;
}

Enemy* FindEnemyByName(WorldBase& world, const std::string& name = "Enemy") {
    return dynamic_cast<Enemy*>(FindAgentByName(world, name));
}

FarmingAgent* FindFarmerByName(WorldBase& world, const std::string& name = "Farmer") {
    return dynamic_cast<FarmingAgent*>(FindAgentByName(world, name));
}

class ScopedInputRedirect {
private:
    std::streambuf* mOldBuffer;
    std::istringstream mInput;

public:
    explicit ScopedInputRedirect(const std::string& data) : mOldBuffer(std::cin.rdbuf()), mInput(data) {
        std::cin.rdbuf(mInput.rdbuf());
    }

    ~ScopedInputRedirect() { std::cin.rdbuf(mOldBuffer); }
};

} // namespace

} // namespace cse498

using namespace cse498;

TEST_CASE("PlayerAgent maps input characters to expected world actions", "[player][input]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    CHECK(player->SelectPlayerAction('w') == WorldActions::MOVE_UP);
    CHECK(player->SelectPlayerAction('W') == WorldActions::MOVE_UP);
    CHECK(player->SelectPlayerAction('s') == WorldActions::MOVE_DOWN);
    CHECK(player->SelectPlayerAction('S') == WorldActions::MOVE_DOWN);
    CHECK(player->SelectPlayerAction('a') == WorldActions::MOVE_LEFT);
    CHECK(player->SelectPlayerAction('A') == WorldActions::MOVE_LEFT);
    CHECK(player->SelectPlayerAction('d') == WorldActions::MOVE_RIGHT);
    CHECK(player->SelectPlayerAction('D') == WorldActions::MOVE_RIGHT);
    CHECK(player->SelectPlayerAction('e') == WorldActions::INTERACT);
    CHECK(player->SelectPlayerAction('E') == WorldActions::INTERACT);
    CHECK(player->SelectPlayerAction('q') == WorldActions::QUIT);
    CHECK(player->SelectPlayerAction('Q') == WorldActions::QUIT);
}

TEST_CASE("PlayerAgent invalid input defaults to remain still", "[player][input]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    CHECK(player->SelectPlayerAction('x') == WorldActions::REMAIN_STILL);
    CHECK(player->SelectPlayerAction('1') == WorldActions::REMAIN_STILL);
    CHECK(player->SelectPlayerAction(' ') == WorldActions::REMAIN_STILL);
}

TEST_CASE("Player can move into an open tile", "[player][movement]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);
    REQUIRE(player->GetLocation().AsWorldPosition() == WorldPosition(2, 2));

    const int result = world.DoAction(*player, WorldActions::MOVE_RIGHT);

    CHECK(result == 1);
    CHECK(player->GetLocation().AsWorldPosition() == WorldPosition(3, 2));
}

TEST_CASE("Player cannot move into a wall", "[player][movement]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    REQUIRE(world.DoAction(*player, WorldActions::MOVE_UP) == 1);
    REQUIRE(player->GetLocation().AsWorldPosition() == WorldPosition(2, 1));

    const int result = world.DoAction(*player, WorldActions::MOVE_UP);

    CHECK(result == 0);
    CHECK(player->GetLocation().AsWorldPosition() == WorldPosition(2, 1));
}

TEST_CASE("Player cannot move into an occupied tile", "[player][movement]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    FarmingAgent* farmer = FindFarmerByName(world);
    REQUIRE(farmer != nullptr);
    REQUIRE(farmer->GetLocation().AsWorldPosition() == WorldPosition(4, 2));

    REQUIRE(world.DoAction(*player, WorldActions::MOVE_RIGHT) == 1);
    REQUIRE(player->GetLocation().AsWorldPosition() == WorldPosition(3, 2));

    const int result = world.DoAction(*player, WorldActions::MOVE_RIGHT);

    CHECK(result == 0);
    CHECK(player->GetLocation().AsWorldPosition() == WorldPosition(3, 2));
    CHECK(farmer->GetLocation().AsWorldPosition() == WorldPosition(4, 2));
}

TEST_CASE("Interact returns 0 when nobody is nearby", "[player][interaction]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    const int result = world.DoAction(*player, WorldActions::INTERACT);

    CHECK(result == 0);
    CHECK(player->GetGold() == 30);
    CHECK(player->GetCurrentHealth() == Approx(100.0));
}

TEST_CASE("Interacting with farmer can cancel trade without changing state", "[player][interaction][trade]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);

    FarmingAgent* farmer = FindFarmerByName(world);
    REQUIRE(farmer != nullptr);

    player->SetLocation(Location(WorldPosition(3, 2)));
    const std::size_t playerGoldBefore = player->GetGold();
    const std::size_t farmerGoldBefore = farmer->GetGold();

    ScopedInputRedirect input("q\n");
    const int result = world.DoAction(*player, WorldActions::INTERACT);

    CHECK(result == 1);
    CHECK(player->GetGold() == playerGoldBefore);
    CHECK(farmer->GetGold() == farmerGoldBefore);
}

TEST_CASE("Player interaction with enemy deals damage both ways when enemy survives", "[player][interaction][combat]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    Enemy* enemy = FindEnemyByName(world);
    REQUIRE(player != nullptr);
    REQUIRE(enemy != nullptr);

    player->SetLocation(Location(WorldPosition(2, 2)));
    enemy->SetLocation(Location(WorldPosition(3, 2)));

    REQUIRE(player->GetGold() == 30);
    REQUIRE(player->GetCurrentHealth() == Approx(100.0));
    REQUIRE(enemy->GetCurrentHealth() == Approx(45.0));

    const int result = world.DoAction(*player, WorldActions::INTERACT);

    CHECK(result == 1);
    CHECK(player->GetCurrentHealth() == Approx(96.0));
    CHECK(enemy->GetCurrentHealth() == Approx(33.0));
    CHECK(player->GetGold() == 30);
    CHECK(player->IsAlive());
    CHECK(enemy->IsAlive());
}

TEST_CASE("Defeating an enemy through player interaction awards gold once", "[player][interaction][combat]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    Enemy* enemy = FindEnemyByName(world);
    REQUIRE(player != nullptr);
    REQUIRE(enemy != nullptr);

    player->SetLocation(Location(WorldPosition(2, 2)));
    enemy->SetLocation(Location(WorldPosition(3, 2)));
    enemy->SetHealth(10.0);

    REQUIRE(player->GetGold() == 30);

    const int result = world.DoAction(*player, WorldActions::INTERACT);

    CHECK(result == 1);
    CHECK_FALSE(enemy->IsAlive());
    CHECK(player->GetGold() == 35);
    CHECK(enemy->ClaimGoldDrop() == 0);
}

TEST_CASE("Quit action ends the demo world", "[player][world]") {
    DemoSimpleWorldG2 world;
    PlayerAgent* player = world.GetPlayer();
    REQUIRE(player != nullptr);
    REQUIRE_FALSE(world.IsRunOver());

    const int result = world.DoAction(*player, WorldActions::QUIT);

    CHECK(result == 1);
    CHECK(world.IsRunOver());
}

TEST_CASE("BuildDiamondNeighborsByRange returns expected tiles for range 2", "[player][interaction][range]") {
    const WorldPosition center(10, 10);
    const auto neighbors = WorldBase::BuildDiamondNeighborsByRange(center, 2);

    // |dx| + |dy| <= 2 excluding center => 12 tiles
    CHECK(neighbors.size() == 12);
    CHECK_FALSE(neighbors.contains(center));

    CHECK(neighbors.contains(WorldPosition(11, 10))); // right 1
    CHECK(neighbors.contains(WorldPosition(10, 9)));  // up 1
    CHECK(neighbors.contains(WorldPosition(10, 11))); // down 1
    CHECK(neighbors.contains(WorldPosition(9, 10)));  // left 1

    CHECK(neighbors.contains(WorldPosition(12, 10))); // right 2
    CHECK(neighbors.contains(WorldPosition(10, 8)));  // up 2
    CHECK(neighbors.contains(WorldPosition(10, 12))); // down 2
    CHECK(neighbors.contains(WorldPosition(8, 10)));  // left 2

    CHECK(neighbors.contains(WorldPosition(11, 9)));  // (+1, -1)
    CHECK(neighbors.contains(WorldPosition(11, 11))); // (+1, +1)
    CHECK(neighbors.contains(WorldPosition(9, 11)));  // (-1, +1)
    CHECK(neighbors.contains(WorldPosition(9, 9)));   // (-1, -1)
}

TEST_CASE("BuildDiamondNeighborsByRange preserves cardinal-first priority order", "[player][interaction][range]") {
    const WorldPosition center(3, 3);
    const auto neighbors = WorldBase::BuildDiamondNeighborsByRange(center, 2);

    REQUIRE(neighbors.at(WorldPosition(4, 3)) == 1); // right 1
    REQUIRE(neighbors.at(WorldPosition(3, 2)) == 2); // up 1
    REQUIRE(neighbors.at(WorldPosition(3, 4)) == 3); // down 1
    REQUIRE(neighbors.at(WorldPosition(2, 3)) == 4); // left 1

    REQUIRE(neighbors.at(WorldPosition(5, 3)) == 5); // right 2
    REQUIRE(neighbors.at(WorldPosition(3, 1)) == 6); // up 2
    REQUIRE(neighbors.at(WorldPosition(3, 5)) == 7); // down 2
    REQUIRE(neighbors.at(WorldPosition(1, 3)) == 8); // left 2

    // Ring-edge non-cardinal points come after cardinals at same distance.
    CHECK(neighbors.at(WorldPosition(4, 2)) > neighbors.at(WorldPosition(5, 3)));
    CHECK(neighbors.at(WorldPosition(4, 4)) > neighbors.at(WorldPosition(5, 3)));
    CHECK(neighbors.at(WorldPosition(2, 4)) > neighbors.at(WorldPosition(5, 3)));
    CHECK(neighbors.at(WorldPosition(2, 2)) > neighbors.at(WorldPosition(5, 3)));
}
