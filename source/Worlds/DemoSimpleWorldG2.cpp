/**
 * Group 2 demo world implementation. Enemy chase logic is defined in AgentFactory
 */
#include "DemoSimpleWorldG2.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "../Agents/Enemy.hpp"
#include "../Agents/FarmingAgent.hpp"
#include "../Agents/PlayerAgent.hpp"
#include "../tools/DamageCalculator.hpp"
#include "Agents/AgentFactory.hpp"

namespace cse498 {

bool DemoSimpleWorldG2::IsOccupiedByAgent(WorldPosition pos, const AgentBase *skip) const {
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        if (skip != nullptr && &GetAgent(i) == skip) {
            continue;
        }
        if (GetAgent(i).GetLocation().AsWorldPosition() == pos) {
            return true;
        }
    }
    return false;
}

void DemoSimpleWorldG2::PrintWorldState() const {
    std::vector<std::string> draw(main_grid.GetHeight(),
                                  std::string(main_grid.GetWidth(), ' '));
    for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
        for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
            draw[y][x] = main_grid.GetCellTypeSymbol(main_grid[WorldPosition{x, y}]);
        }
    }
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        const AgentBase &agent = GetAgent(i);
        const WorldPosition pos = agent.GetLocation().AsWorldPosition();
        draw[pos.CellY()][pos.CellX()] = agent.GetSymbol();
    }
    std::cout << '\n' << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    for (const std::string &row : draw) {
        std::cout << "|" << row << "|\n";
    }
    std::cout << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    std::cout << "Player HP: " << static_cast<int>(GetAgent(kPlayerIdx).GetHealth())
              << " | Enemy HP: " << static_cast<int>(GetAgent(kEnemyIdx).GetHealth()) << '\n';
}

bool DemoSimpleWorldG2::MoveAgentBy(AgentBase &agent, double dx, double dy) {
    const WorldPosition current = agent.GetLocation().AsWorldPosition();
    const WorldPosition next = current.GetOffset(dx, dy);
    if (!main_grid.IsValid(next)) {
        return false;
    }
    if (main_grid[next] == mWallId) {
        return false;
    }
    if (IsOccupiedByAgent(next, &agent)) {
        return false;
    }
    agent.SetLocation(Location(next));
    return true;
}

int DemoSimpleWorldG2::HandleInteraction(AgentBase &actor) {
    const WorldPosition actor_pos = actor.GetLocation().AsWorldPosition();
    bool interacted = false;

    for (size_t i = 0; i < GetNumAgents(); ++i) {
        if (&GetAgent(i) == &actor) {
            continue;
        }
        AgentBase &other = GetAgent(i);
        const WorldPosition other_pos = other.GetLocation().AsWorldPosition();
        const double dx = std::abs(actor_pos.X() - other_pos.X());
        const double dy = std::abs(actor_pos.Y() - other_pos.Y());
        if (dx <= 1.0 && dy <= 1.0) {
            if (i == kFarmerIdx && &actor == &GetAgent(kEnemyIdx)) {
                continue;
            }
            interacted = true;
            if (i == kFarmerIdx) {
                auto &farmer = dynamic_cast<FarmingAgent &>(other);
                std::cout << "Farmer: ";
                if (farmer.IsAvailableForTrade()) {
                    std::cout << "Welcome! I've got crops and seeds today.\n";
                } else {
                    std::cout << "Sorry, I'm not trading right now.\n";
                }
            } else if (i == kPlayerIdx && &actor == &GetAgent(kEnemyIdx)) {
                const double dealt =
                    DamageCalculator::Calculate(mEnemyCombat, mPlayerCombat);
                other.TakeDamage(dealt);
                std::cout << actor.GetName() << " hits " << other.GetName() << " for "
                          << static_cast<int>(dealt) << " damage.\n";
                if (!other.IsAlive()) {
                    std::cout << other.GetName() << " has fallen.\n";
                    run_over = true;
                    return 1;
                }
                const double retaliate =
                    DamageCalculator::Calculate(mPlayerCombat, mEnemyCombat);
                actor.TakeDamage(retaliate);
                std::cout << other.GetName() << " strikes back for " << static_cast<int>(retaliate)
                          << " damage.\n";
                if (!actor.IsAlive()) {
                    std::cout << "Enemy defeated.\n";
                    run_over = true;
                    return 1;
                }
            } else if (i == kEnemyIdx) {
                const double dealt =
                    DamageCalculator::Calculate(mPlayerCombat, mEnemyCombat);
                other.TakeDamage(dealt);
                std::cout << actor.GetName() << " hits enemy for " << static_cast<int>(dealt)
                          << " damage.\n";
                if (!other.IsAlive()) {
                    std::cout << "Enemy defeated.\n";
                    run_over = true;
                    return 1;
                }
                const double retaliate =
                    DamageCalculator::Calculate(mEnemyCombat, mPlayerCombat);
                actor.TakeDamage(retaliate);
                std::cout << "Enemy strikes back for " << static_cast<int>(retaliate) << " damage.\n";
                if (!actor.IsAlive()) {
                    std::cout << actor.GetName() << " has fallen.\n";
                    run_over = true;
                    return 1;
                }
            }
        }
    }
    if (!interacted) {
        std::cout << "No one nearby to interact with.\n";
    }
    return interacted ? 1 : 0;
}

void DemoSimpleWorldG2::ConfigAgent(AgentBase &agent) {
    namespace A = DemoSimpleWorldG2Actions;
    agent.AddAction("up", A::MOVE_UP);
    agent.AddAction("down", A::MOVE_DOWN);
    agent.AddAction("left", A::MOVE_LEFT);
    agent.AddAction("right", A::MOVE_RIGHT);
    agent.AddAction("interact", A::INTERACT);
    agent.AddAction("quit", A::QUIT);
}

DemoSimpleWorldG2::DemoSimpleWorldG2() {
    mFloorId = main_grid.AddCellType("floor", "Walkable floor", '.');
    mWallId = main_grid.AddCellType("wall", "Solid wall", '#');
    main_grid.Load({
        "############",
        "#..........#",
        "#..........#",
        "#..........#",
        "#..........#",
        "############",
    });


    auto* player = GetPlayer();
    DemoSimpleWorldG2::ConfigAgent(*player);
    player->SetSymbol('@');
    player->SetMaxHealth(100.0);
    player->SetHealth(100.0);
    player->SetLocation(Location(WorldPosition{2, 2}));
    mPlayerCombat = CombatStats{14.0, 5.0};

    auto &farmer = AddAgent<FarmingAgent>("Farmer");
    farmer.SetSymbol('F');
    farmer.SetLocation(Location(WorldPosition{4, 2}));

    auto &enemy = AddAgent<Enemy>("Enemy");
    enemy.SetSymbol('S');
    enemy.SetMaxHealth(45.0);
    enemy.SetHealth(45.0);
    enemy.SetLocation(Location(WorldPosition{8, 3}));
    mEnemyCombat = CombatStats{9.0, 2.0};
    enemy.SetBehaviorTree(AgentFactory::CreateEnemyFollowPlayerTree(&enemy, *this, kPlayerIdx));
}

int DemoSimpleWorldG2::DoAction(AgentBase &agent, size_t action_id) {
    namespace A = DemoSimpleWorldG2Actions;
    if (action_id == A::QUIT) {
        run_over = true;
        std::cout << "Quitting demo.\n";
        return 1;
    }
    switch (action_id) {
    case A::MOVE_UP:
        return MoveAgentBy(agent, 0.0, -1.0);
    case A::MOVE_DOWN:
        return MoveAgentBy(agent, 0.0, 1.0);
    case A::MOVE_LEFT:
        return MoveAgentBy(agent, -1.0, 0.0);
    case A::MOVE_RIGHT:
        return MoveAgentBy(agent, 1.0, 0.0);
    case A::INTERACT:
        return HandleInteraction(agent);
    case A::REMAIN_STILL:
    default:
        return 0;
    }
}

void DemoSimpleWorldG2::Run() {
    run_over = false;
    while (!run_over) {
        PrintWorldState();
        PlayerAgent* player = GetPlayer();
        if (player == nullptr) return;

        if (!player->IsAlive()) {
            break;
        }
        const size_t action_id = player->SelectAction(main_grid);
        const int result = DoAction(*player, action_id);
        player->SetActionResult(result);
        if (run_over) {
            break;
        }
        AgentBase &enemy = GetAgent(kEnemyIdx);
        if (enemy.IsAlive()) {
            const size_t enemy_action = enemy.SelectAction(main_grid);
            const int enemy_result = DoAction(enemy, enemy_action);
            enemy.SetActionResult(enemy_result);
        }
    }
    PrintWorldState();
    std::cout << "Demo complete.\n";
}

} // namespace cse498
