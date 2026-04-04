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
        const AgentBase& agent = GetAgent(i);

        if (&agent == skip)
        {
            continue;
        }

        if (!agent.IsAlive())
        {
            continue;
        }

        if (agent.GetLocation().AsWorldPosition() == pos) {
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
        if (!agent.IsAlive())
        {
            continue;
        }
        const WorldPosition pos = agent.GetLocation().AsWorldPosition();
        draw[pos.CellY()][pos.CellX()] = agent.GetSymbol();
    }
    std::cout << '\n' << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    for (const std::string &row : draw) {
        std::cout << "|" << row << "|\n";
    }
    std::cout << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    const PlayerAgent *player = GetPlayer();
    const AgentBase &enemy = GetAgent(kEnemyIdx);
    std::cout << "Player HP: " << static_cast<int>(GetAgent(kPlayerIdx).GetHealth())
              << " | Player Gold: " << (player ? player->GetGold() : 0);
    if (enemy.IsAlive()) {
        std::cout << " | Enemy HP: " << static_cast<int>(enemy.GetHealth());
    } else {
        std::cout << " | Enemy defeated";
    }
    std::cout << '\n';
}

bool DemoSimpleWorldG2::MoveAgentBy(AgentBase &agent, double dx, double dy) {
    // There needs to be a function like this in the WorldBase Class -- this is just an example implementation
    // for this class. Recall for 8-directional movement you need to check that the two adjacent tiles are free
    // to move in a diagonal direction (not done here).
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
        if (!other.IsAlive())
        {
            continue;
        }
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
                if (&actor == GetPlayer()) {
                    return HandleMerchantTrade(farmer);
                }

                std::cout << "Farmer: ";
                if (farmer.IsAvailableForTrade()) {
                    std::cout << farmer.GetTradeGreeting() << '\n';
                } else {
                    std::cout << farmer.GetTradeClosedMessage() << '\n';
                }
            } else if (i == kPlayerIdx && &actor == &GetAgent(kEnemyIdx)) {
                auto &player = dynamic_cast<PlayerAgent &>(other);
                auto &enemy = dynamic_cast<Enemy &>(actor);

                const double dealt =
                    DamageCalculator::Calculate(mEnemyCombat, mPlayerCombat);
                player.TakeDamage(dealt);
                std::cout << enemy.GetName() << " hits " << player.GetName() << " for "
                          << static_cast<int>(dealt) << " damage.\n";
                if (!player.IsAlive()) {
                    std::cout << player.GetName() << " has fallen.\n";
                    run_over = true;
                    return 1;
                }
                const double retaliate =
                    DamageCalculator::Calculate(mPlayerCombat, mEnemyCombat);
                enemy.TakeDamage(retaliate);
                std::cout << player.GetName() << " strikes back for " << static_cast<int>(retaliate)
                          << " damage.\n";
                if (!enemy.IsAlive()) {
                    HandleEnemyDefeat(enemy, player);
                    return 1;
                }
            } else if (i == kEnemyIdx) {
                const double dealt =
                    DamageCalculator::Calculate(mPlayerCombat, mEnemyCombat);
                other.TakeDamage(dealt);
                std::cout << actor.GetName() << " hits enemy for " << static_cast<int>(dealt)
                          << " damage.\n";
                if (!other.IsAlive()) {
                    auto &enemy = dynamic_cast<Enemy &>(other);
                    auto &player = dynamic_cast<PlayerAgent &>(actor);
                    HandleEnemyDefeat(enemy, player);
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

int DemoSimpleWorldG2::HandleMerchantTrade(MerchantAgent& merchant) const
{
    PlayerAgent *player = GetPlayer();
    if (player == nullptr) {
        return 0;
    }

    std::cout << "\n=== Merchant Trade ===\n";
    std::cout << merchant.GetName() << ": " << merchant.GetTradeGreeting() << '\n';
    std::cout << "Your gold: " << player->GetGold() << '\n';
    std::cout << "Merchant gold: " << merchant.GetGold() << '\n';

    std::cout << "\nYour inventory:\n";
    std::cout << player->GetInventory() << '\n';

    std::cout << "\nShop offers:\n";
    for (const TradeOffer &offer : merchant.GetOffers()) {
        std::cout << " - " << offer.mItemName
                  << " | buy: " << offer.mBuyPrice
                  << " | sell: " << offer.mSellPrice
                  << " | stock: ";
        if (offer.IsUnlimited()) {
            std::cout << "unlimited";
        } else {
            std::cout << offer.mStock;
        }
        std::cout << '\n';
    }

    std::cout << "\nChoose an action: [b] buy  [s] sell  [q] cancel\n> ";

    char choice = '\0';
    std::cin >> choice;

    if (choice == 'q' || choice == 'Q') {
        std::cout << "Trade cancelled.\n";
        return 1;
    }

    if (choice != 'b' && choice != 'B' && choice != 's' && choice != 'S')
    {
        std::cout << "Invalid trade option.\n";
        return 0;
    }

    std::string itemName;
    std::size_t quantity = 1;

    std::cout << "Item name: ";
    std::cin >> itemName;
    std::cout << "Quantity: ";
    std::cin >> quantity;

    if (quantity == 0)
    {
        std::cout << "Quantity must be at least 1.\n";
        return 0;
    }

    TradeResult result;
    if (choice == 'b' || choice == 'B')
    {
        result = merchant.BuyFromMerchant(*player, itemName, quantity);
    }
    else
    {
        result = merchant.SellToMerchant(*player, itemName, quantity);
    }

    std::cout << result.mMessage << '\n';
    std::cout << "Your gold is now: " << player->GetGold() << '\n';
    std::cout << "Merchant gold is now: " << merchant.GetGold() << '\n';

    if (const TradeOffer* updatedOffer = merchant.FindOffer(itemName); updatedOffer != nullptr)
    {
        std::cout << "Updated shop entry for " << updatedOffer->mItemName << ": stock = ";
        if (updatedOffer->IsUnlimited())
        {
            std::cout << "unlimited";
        }
        else
        {
            std::cout << updatedOffer->mStock;
        }
        std::cout << '\n';
    }

    return result.IsSuccess() ? 1 : 0;
}

void DemoSimpleWorldG2::HandleEnemyDefeat(Enemy& enemy, PlayerAgent& player)
{
    const std::size_t goldReward = enemy.ClaimGoldDrop();

    std::cout << "Enemy defeated.\n";

    if (goldReward > 0) {
        player.AddGold(goldReward);
        std::cout << player.GetName() << " gains " << goldReward << " gold.\n";
    }
}

void DemoSimpleWorldG2::ConfigAgent(AgentBase &agent) {
    namespace A = DemoSimpleWorldG2Actions;
    agent.AddAction("w", A::MOVE_UP);
    agent.AddAction("s", A::MOVE_DOWN);
    agent.AddAction("a", A::MOVE_LEFT);
    agent.AddAction("d", A::MOVE_RIGHT);
    agent.AddAction("e", A::INTERACT);
    agent.AddAction("q", A::QUIT);
    agent.AddAction("stay", A::REMAIN_STILL);
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
    // Need to call this function to ensure player is set up for this world.
    DemoSimpleWorldG2::ConfigAgent(*player);
    player->SetSymbol('@');
    player->SetMaxHealth(100.0);
    player->SetHealth(100.0);
    player->SetLocation(Location(WorldPosition{2, 2}));
    player->SetGold(30);
    mPlayerCombat = CombatStats{14.0, 5.0};

    auto &farmer = AddAgent<FarmingAgent>("Farmer");
    farmer.SetSymbol('F');
    farmer.SetLocation(Location(WorldPosition{4, 2}));
    farmer.ClearInitialOffers();
    farmer.AddInitialOffer({"apple", 4, 2, 1, TradeStockMode::Unlimited, 0});
    farmer.AddInitialOffer({"bread", 6, 3, 1, TradeStockMode::Limited, 18});
    farmer.AddInitialOffer({"potion", 10, 5, 1, TradeStockMode::Limited, 10});
    farmer.AddGold(200);

    // just for demonstration of another method for creation
    auto &enemy = AddAgent(std::make_unique<Enemy>(GetNextAgentId(), "Enemy", *this));
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

        std::cout << "WASD move, E interact with NPC/enemy, X quit.\n> ";
        char input;
        std::cin >> input;
        const size_t action_id = player->SelectPlayerAction(input);
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
