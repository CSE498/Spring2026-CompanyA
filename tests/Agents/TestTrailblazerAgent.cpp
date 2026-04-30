/**
 * @file TestTrailblazerAgent.cpp
 * @brief Catch2 unit tests for TrailblazerAgent (AIWorld integration).
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/Agents/AI/TrailblazerAgent.hpp"
#include "../../source/Agents/AI/EnemyAgent.hpp"
#include "../../source/Worlds/DemoG1/AIWorld.hpp"
#include "../../source/Worlds/MazeWorld.hpp"

using namespace cse498;

namespace {
struct AIWorldStateHarness : AIWorld {
public:
    using AIWorld::mAgentState;
    using AIWorld::mEnemies;
};

struct InteractionWorld : MazeWorld {
    void ConfigAgent(AgentBase& agent) override {
        MazeWorld::ConfigAgent(agent);
        agent.AddAction("e", 5);
        agent.AddAction("interact", 5);
    }
};
} // namespace

TEST_CASE("TrailblazerAgent Initialize sets symbol and succeeds", "[TrailblazerAgent]") {
    AIWorld world;
    auto& agent = world.AddAgent<TrailblazerAgent>("Hero");
    CHECK(agent.Initialize());
    CHECK(agent.GetSymbol() == 'T');
}

TEST_CASE("TrailblazerAgent attacks adjacent enemy to the east", "[TrailblazerAgent]") {
    AIWorld world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);

    auto& hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{4, 5});

    size_t action = hero.SelectAction(world.GetGrid());
    CHECK(action == hero.GetActionID("attack_right"));
}

TEST_CASE("TrailblazerAgent returns interact action when enemy is adjacent outside AIWorld", "[TrailblazerAgent]") {
    InteractionWorld world;

    auto& trailblazer = world.AddAgent<TrailblazerAgent>("Trailblazer");
    trailblazer.SetLocation(WorldPosition{10, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{11, 7});

    size_t action = trailblazer.SelectAction(world.GetGrid());

    CHECK(action == 5);
}

TEST_CASE("TrailblazerAgent attacks adjacent enemy in all cardinal directions", "[TrailblazerAgent]") {
    AIWorld world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);

    struct Case {
        WorldPosition agent;
        const char* expected_attack;
    };
    // Default enemy #0 sits at (5, 5).
    const Case cases[] = {
            {WorldPosition{5, 4}, "attack_down"},
            {WorldPosition{5, 6}, "attack_up"},
            {WorldPosition{4, 5}, "attack_right"},
            {WorldPosition{6, 5}, "attack_left"},
    };

    for (const auto& c: cases) {
        AIWorld w;
        w.SetVerboseMode(false);
        w.SetStepMode(false);
        auto& a = w.AddAgent<TrailblazerAgent>("Hero");
        a.SetLocation(c.agent);
        size_t act = a.SelectAction(w.GetGrid());
        CHECK(act == a.GetActionID(c.expected_attack));
    }
}

TEST_CASE("LearningExplorerAgent interacts with diagonal adjacent enemy outside of AI World", "[LearningExplorerAgent]") {
    InteractionWorld world;

    auto& explorer = world.AddAgent<TrailblazerAgent>("Explorer");
    explorer.SetLocation(WorldPosition{10, 7});

    auto& enemy = world.AddAgent<EnemyAgent>("Enemy");
    enemy.SetLocation(WorldPosition{11, 8});

    size_t action = explorer.SelectAction(world.GetGrid());

    CHECK(action == 5);
}

TEST_CASE("TrailblazerAgent picks up loot when standing on item tile", "[TrailblazerAgent]") {
    AIWorld world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);

    auto& hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{3, 5});

    size_t action = hero.SelectAction(world.GetGrid());
    CHECK(action == hero.GetActionID("pickup"));
}

TEST_CASE("TrailblazerAgent plans use_heal when hurt and holding charges", "[TrailblazerAgent]") {
    AIWorldStateHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);

    auto& hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});

    auto& st = world.mAgentState.at(hero.GetID());
    st.mHP = 3;
    st.mMaxHP = 12;
    st.mHealCharges = 4;

    size_t action = hero.SelectAction(world.GetGrid());
    CHECK(action == hero.GetActionID("use_heal"));
}

TEST_CASE("TrailblazerAgent explores when all enemies are defeated", "[TrailblazerAgent]") {
    AIWorldStateHarness world;
    world.SetVerboseMode(false);
    world.SetStepMode(false);

    for (auto& enemy: world.mEnemies) {
        enemy.mAlive = false;
    }

    auto& hero = world.AddAgent<TrailblazerAgent>("Hero");
    hero.SetLocation(WorldPosition{10, 7});

    size_t action = hero.SelectAction(world.GetGrid());
    CHECK(action != 0);
    const bool is_move = (action == hero.GetActionID("up")) || (action == hero.GetActionID("down")) ||
                         (action == hero.GetActionID("left")) || (action == hero.GetActionID("right"));
    CHECK(is_move);
}
