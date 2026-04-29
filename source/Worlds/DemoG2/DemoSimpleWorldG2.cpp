/**
 * Group 2 demo world implementation. Enemy chase logic is defined in AgentFactory
 */
#include "DemoSimpleWorldG2.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "../../Agents/Classic/AgentFactory.hpp"
#include "../../Agents/Classic/Enemy.hpp"
#include "../../Agents/Classic/FarmingAgent.hpp"
#include "../../Agents/Classic/PlayerAgent.hpp"
#include "../../tools/DamageCalculator.hpp"
#include "WorldActions.hpp"
#include "core/item/ItemWeaponSword.hpp"
#include "core/item/Item.hpp"

namespace cse498 {


bool DemoSimpleWorldG2::IsOccupiedByAgent(WorldPosition pos, const AgentBase* skip) const {
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        const AgentBase& agent = GetAgentByIndex(i);

        if (&agent == skip) {
            continue;
        }

        if (!agent.IsAlive()) {
            continue;
        }

        if (agent.GetLocation().AsWorldPosition() == pos) {
            return true;
        }
    }
    return false;
}

void DemoSimpleWorldG2::PrintWorldState() const {
    std::vector<std::string> draw(main_grid.GetHeight(), std::string(main_grid.GetWidth(), ' '));
    for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
        for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
            draw[y][x] = main_grid.GetCellTypeSymbol(main_grid[WorldPosition{x, y}]);
        }
    }
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        const AgentBase& agent = GetAgentByIndex(i);
        if (!agent.IsAlive()) {
            continue;
        }
        const WorldPosition pos = agent.GetLocation().AsWorldPosition();
        draw[pos.CellY()][pos.CellX()] = agent.GetSymbol();
    }
    std::cout << '\n' << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    for (const std::string& row: draw) {
        std::cout << "|" << row << "|\n";
    }
    std::cout << '+' << std::string(main_grid.GetWidth(), '-') << "+\n";
    const PlayerAgent* player = GetPlayer();
    if (!player) {
        std::cout << "player is dead/gone";
        return;
    }

    const AgentBase* enemy = TryGetAgent(mEnemyId);
    std::cout << "Player HP: " << player->GetCurrentHealth()
              << " | Player Gold: " << player->GetGold() << " | Atk: " << player->GetAtk() << " | Range: " << player->GetAtkRange();
    if (enemy != nullptr && enemy->IsAlive()) {
        std::cout << " | Enemy HP: " << static_cast<int>(enemy->GetCurrentHealth());
    } else {
        std::cout << " | Enemy defeated";
    }
    std::cout << '\n';

    auto* item = player->GetInventory().GetHand();
    if (item)
        std::cout << "Current Hand Item: " << *item << std::endl; // (GetPlayer() ? GetPlayer()->GetInventory().GetHand() : "");
    else
        std::cout << "Current Hand Item: none" << std::endl; // (GetPlayer() ? GetPlayer()->GetInventory().GetHand() : "");



}

bool DemoSimpleWorldG2::MoveAgentBy(AgentBase& agent, double dx, double dy) {
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

int DemoSimpleWorldG2::HandleInteraction(AgentBase& actor) {

    /*
     * PLEASE READ
     * NOTE NOTE NOTE
     *
     * This function was only ever supposed to do output of text on what was happening in the interaction
     * -- A Wrapper for the interaction
     *
     * But it instead makes up for the lack of implementation with interacting with the farmer -
     * We never built a system for doing "b" to buy from trader and UI interface part of trading so
     * That is instead for this as this file is an integrated world + interface
     *
     * but it has to because it makes up for the lack of implementation
     * structure elsewhere. It was just hard to get everyone on board with timing and all to know
     *
     *
     * NOTE NOTE NOTE
     */


    const WorldPosition actor_pos = actor.GetLocation().AsWorldPosition();
    bool interacted = false;
    // Assume actor is player
    if (actor.GetID() != GetPlayer()->GetID())
        return 0;

    for (size_t i = 0; i < GetNumAgents(); ++i) {
        AgentBase& other = GetAgentByIndex(i);


        if (&other == &actor) {
            continue;
        }
        if (!other.IsAlive()) {
            continue;
        }


        const WorldPosition other_pos = other.GetLocation().AsWorldPosition();
        const double dx = std::abs(actor_pos.X() - other_pos.X());
        const double dy = std::abs(actor_pos.Y() - other_pos.Y());
        // Interaction should commence between the two parties
        // To move this into the behavior tree of agents we need a way to find enemies in radius R
        // In practice this will be moved and handled by us, not the world.
        if (dx <= 1.0 && dy <= 1.0 && other.GetID() == mFarmerId)
        {
            auto& farmer = dynamic_cast<FarmingAgent&>(other);
            interacted = true;
            if (&actor == GetPlayer()) {
                return HandleMerchantTrade(farmer);
            }
            std::cout << "Farmer: ";
            if (farmer.IsAvailableForTrade()) {
                std::cout << farmer.GetTradeGreeting() << '\n';
            } else {
                std::cout << farmer.GetTradeClosedMessage() << '\n';
            }
            return interacted;
        }
        // otherwise just act as if it is an enemy
        // This is ELSE case:

        auto& player = dynamic_cast<PlayerAgent&>(actor); // we know this is true from start of func
        if (!other.IsEnemy())
            continue;
        auto& enemy = dynamic_cast<Enemy&>(other); // true

        interacted = Interact(); // this will handle the interaction finding the enemy nearby. We TERMINATE ASAP now

        /*
         * This is just ouputting information now based on the interaction
         */

        std::cout << player.GetName() << " hits " << enemy.GetName() << " for " << player.GetLastDamageDealt()
          << " damage.\n";

        if (!enemy.IsAlive()) {
            HandleEnemyDefeat(enemy, player);
            return 1;
        }

        if (interacted)
            return true;
    }

    // No interaction
    std::cout << "No one nearby to interact with.\n";
    return false;
}

int DemoSimpleWorldG2::HandleMerchantTrade(MerchantAgent& merchant) const {
    PlayerAgent* player = GetPlayer();
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
    for (const TradeOffer& offer: merchant.GetOffers()) {
        std::cout << " - " << offer.mItemName << " | buy: " << offer.mBuyPrice << " | sell: " << offer.mSellPrice
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

    if (choice != 'b' && choice != 'B' && choice != 's' && choice != 'S') {
        std::cout << "Invalid trade option.\n";
        return 0;
    }

    std::string itemName;
    std::size_t quantity = 1;

    std::cout << "Item name: ";
    std::cin >> itemName;
    std::cout << "Quantity: ";
    std::cin >> quantity;

    if (quantity == 0) {
        std::cout << "Quantity must be at least 1.\n";
        return 0;
    }

    TradeResult result;
    if (choice == 'b' || choice == 'B') {
        result = merchant.BuyFromMerchant(*player, itemName, quantity);
    } else {
        result = merchant.SellToMerchant(*player, itemName, quantity);
    }

    std::cout << result.mMessage << '\n';
    std::cout << "Your gold is now: " << player->GetGold() << '\n';
    std::cout << "Merchant gold is now: " << merchant.GetGold() << '\n';

    if (const TradeOffer* updatedOffer = merchant.FindOffer(itemName); updatedOffer != nullptr) {
        std::cout << "Updated shop entry for " << updatedOffer->mItemName << ": stock = ";
        if (updatedOffer->IsUnlimited()) {
            std::cout << "unlimited";
        } else {
            std::cout << updatedOffer->mStock;
        }
        std::cout << '\n';
    }

    return result.IsSuccess() ? 1 : 0;
}

void DemoSimpleWorldG2::HandleEnemyDefeat(Enemy& enemy, PlayerAgent& player) {
    const std::size_t goldReward = enemy.ClaimGoldDrop();

    std::cout << "Enemy defeated.\n";

    if (goldReward > 0) {
        player.AddGold(goldReward);
        std::cout << player.GetName() << " gains " << goldReward << " gold.\n";
    }
}

void DemoSimpleWorldG2::ConfigAgent(AgentBase& agent) {
    agent.AddAction(WorldActions::MOVE_UP_STRING, WorldActions::MOVE_UP);
    agent.AddAction(WorldActions::MOVE_DOWN_STRING, WorldActions::MOVE_DOWN);
    agent.AddAction(WorldActions::MOVE_LEFT_STRING, WorldActions::MOVE_LEFT);
    agent.AddAction(WorldActions::MOVE_RIGHT_STRING, WorldActions::MOVE_RIGHT);
    agent.AddAction(WorldActions::INTERACT_STRING, WorldActions::INTERACT);
    agent.AddAction(WorldActions::QUIT_STRING, WorldActions::QUIT);
    agent.AddAction(WorldActions::REMAIN_STILL_STRING, WorldActions::REMAIN_STILL);
}

DemoSimpleWorldG2::DemoSimpleWorldG2() {

    auto p = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this); // ID = 0
    AddAgent(std::move(p));
    mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
    assert(mPlayer);
    mPlayer->GetInventory().AddItem(
    std::make_unique<ItemWeaponSword>(
        0,
        "sword",
        "/assets/items/item_sword_1.png",
        3,
        *this
    ));
    // Just a quick test to make sure it is there -- Adding an item to 2nd slot of hotbar - get there with inc.
    mPlayer->GetInventory().SwapSlots(Inventory::HOTBAR_SIZE, 1);
    mPlayer->GetInventory().HotBarIndexInc();
    [[maybe_unused]] auto x = mPlayer->GetInventory().GetHand();
    assert(x->GetName() == "sword" && "Sword is not correct");
    mPlayer->GetInventory().HotBarIndexDec();



    mFloorId = main_grid.AddCellType("floor", "Walkable floor", '.');
    mWallId = main_grid.AddCellType("wall", "Solid wall", '#');
    main_grid.Load({
            "############", // 10x10 grid --> I know it looks vertical but 10x10
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "#..........#",
            "############",
    });


    auto* player = GetPlayer();
    mPlayerId = player->GetID();
    // Need to call this function to ensure player is set up for this world.
    // DemoSimpleWorldG2::ConfigAgent(*player);
    player->SetSymbol('@');
    player->SetStats(AgentStats(100, 14, 5, 3, 0));
    player->SetLocation(Location(WorldPosition{3, 4}));
    player->SetGold(30);

    // Creating agents by using the template this way
    auto& farmer = AddAgent<FarmingAgent>("Farmer");
    mFarmerId = farmer.GetID(); // ID = 1
    farmer.SetSymbol('F');
    farmer.SetLocation(Location(WorldPosition{3, 1}));
    farmer.ClearInitialOffers();
    farmer.AddInitialOffer({"apple", 4, 2, 1, TradeStockMode::Unlimited, 0});
    farmer.AddInitialOffer({"bread", 6, 3, 1, TradeStockMode::Limited, 18});
    farmer.AddInitialOffer({"potion", 10, 5, 1, TradeStockMode::Limited, 10});
    farmer.AddGold(200);

    CreateEnemies(1);
    // just for demonstration of another method for creation of an agent

}

int DemoSimpleWorldG2::DoAction(AgentBase& agent, size_t action_id) {
    if (action_id == WorldActions::QUIT) {
        mRunOver = true;
        std::cout << "Quitting demo.\n";
        return 1;
    }
    switch (action_id) {
        case WorldActions::MOVE_UP:
            return MoveAgentBy(agent, 0.0, -1.0);
        case WorldActions::MOVE_DOWN:
            return MoveAgentBy(agent, 0.0, 1.0);
        case WorldActions::MOVE_LEFT:
            return MoveAgentBy(agent, -1.0, 0.0);
        case WorldActions::MOVE_RIGHT:
            return MoveAgentBy(agent, 1.0, 0.0);
        case WorldActions::INTERACT:
            return HandleInteraction(agent);
        case WorldActions::REMAIN_STILL:
        default:
            return 0;
    }
}

void DemoSimpleWorldG2::Run() {
    mRunOver = false;
    while (!mRunOver) {
        PrintWorldState();
        PlayerAgent* player = GetPlayer();
        if (player == nullptr)
            return;

        // completion on death of player.
        if (!player->IsAlive()) {
            break;
        }

        std::cout << "WASD move, E interact with NPC/enemy, Q quit. \n "
                     "('_' or '-') Inv Scroll Down, ('+', '=') Inv scroll up, I see Inv \n> ";

        // Just read the whole line and take the first char in case there were other garbage values.
        // One turn at a time is more consistent to test
        std::string line;
        std::getline(std::cin, line);
        char input = line.empty() ? '\0' : line[0];

        auto scrollTest = IsScroll(input);
        if (scrollTest) // just handle the scroll
        {
            (scrollTest.value()) ? mPlayer->GetInventory().HotBarIndexInc() : mPlayer->GetInventory().HotBarIndexDec();
            continue;
        }
        if (input == 'i' || input == 'I')
        {
            std::cout << player->GetInventory(); // nothing could've happened between here and previous check for null
            continue;
        }


        const size_t action_id = player->SelectPlayerAction(input);
        const int result = DoAction(*player, action_id);
        player->SetActionResult(result);
        RemoveDeadAgents();

        if (mRunOver) {
            break;
        }

        AgentBase* enemy = TryGetAgent(mEnemyId);
        if (enemy != nullptr && enemy->IsAlive()) {
            const size_t enemy_action = enemy->SelectAction(main_grid);
            const int enemy_result = DoAction(*enemy, enemy_action);
            enemy->SetActionResult(enemy_result);
            RemoveDeadAgents();
        }
    }
    PrintWorldState();
    std::cout << "Demo complete.\n";
}


std::optional<bool> DemoSimpleWorldG2::IsScroll(char s)
{
    switch (s)
    {
        case '-':
        case '_':
            return false;
        case '+':
        case '=':
            return true;
        default: return {};
    }
}


void DemoSimpleWorldG2::CreateEnemies(int option)
{
    if (option == 0) // new setup
    {
        auto skel = AgentFactory::CreateEnemySkeleton({"Enemy", 0, {9,3}}, *this);
        auto& enemy = AddAgent(std::move(skel));
        mEnemyId = enemy.GetID();
        assert(mEnemyId == 2);
        enemy.SetSymbol('S');

    }
    else if (option == 1) // old setup
    {
        GetPlayer()->SetPosition({2,2});
        // index != id in general but here for initialization it is fine.
        // Should have a GetAgentById function but wouldn't be really useful aside from right here.
        GetAgentByIndex(mFarmerId).SetPosition({4,2});

        mEnemyId = GetNextAgentId(); // this is just for demonstration. ids should be organized by world ID = 2
        auto& enemy = AddAgent(std::make_unique<Enemy>(mEnemyId, "Enemy", *this));
        enemy.SetLocation(Location(WorldPosition{8, 3}));
        enemy.SetStats(AgentStats(45, 9, 2, 3, 0));
        enemy.SetBehaviorTree(AgentFactory::CreateEnemyFollowPlayerTree(&enemy, *this, mPlayerId));
        enemy.SetSymbol('S');

    }

}

} // namespace cse498
