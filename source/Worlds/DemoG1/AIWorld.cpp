#include "AIWorld.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <memory>

namespace cse498 {

LootItem::LootItem(size_t id, const std::string& name, const WorldBase& world, ItemKind kind, int power) :
    ItemBase(id, name, world), mKind(kind), mPower(power) {}

LootItem::ItemKind LootItem::GetKind() const { return mKind; }

int LootItem::GetPower() const { return mPower; }

bool LootItem::IsConsumed() const { return mConsumed; }

void LootItem::Consume() { mConsumed = true; }

bool AIWorld::WaitForStep() {
    if (!mStepMode)
        return true;
    std::cout << "Press Enter for next step (q to quit)...";
    std::cout.flush();
    std::string line;
    std::getline(std::cin, line);
    size_t i = 0;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])))
        ++i;
    if (i >= line.size())
        return true;
    if (line[i] != 'q' && line[i] != 'Q')
        return true;
    ++i;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])))
        ++i;
    if (i < line.size())
        return true;
    std::cout << "Quit requested.\n";
    return false;
}

std::string AIWorld::ActionName(size_t action_id) const {
    switch (action_id) {
        case 0:
            return "remain";
        case 1:
            return "move_up";
        case 2:
            return "move_down";
        case 3:
            return "move_left";
        case 4:
            return "move_right";
        case PICKUP_ITEM:
            return "pickup";
        case USE_HEAL:
            return "use_heal";
        case ATTACK_UP:
            return "attack_up";
        case ATTACK_DOWN:
            return "attack_down";
        case ATTACK_LEFT:
            return "attack_left";
        case ATTACK_RIGHT:
            return "attack_right";
        default:
            return "unknown";
    }
}

void AIWorld::PrintWorldState(const std::string& header) const {
    if (!mVerboseMode)
        return;
    std::cout << "\n=== " << header << " (Turn " << mCurrentTurn << ") ===\n";
    for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
        for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
            const WorldPosition pos{x, y};
            char symbol = main_grid.GetCellTypeSymbol(main_grid[pos]);

            for (const auto& item_ptr: item_set) {
                const auto* loot_item = dynamic_cast<const LootItem*>(item_ptr.get());
                if (!loot_item || loot_item->IsConsumed() || !loot_item->GetLocation().IsPosition())
                    continue;
                if (loot_item->GetLocation().AsWorldPosition() == pos) {
                    symbol = (loot_item->GetKind() == LootItem::ItemKind::Weapon) ? 'W' : 'H';
                }
            }

            for (const auto& enemy: mEnemies) {
                if (enemy.mAlive && enemy.mPosition == pos)
                    symbol = 'X';
            }

            for (size_t a = 0; a < GetNumAgents(); ++a) {
                const AgentBase& agent = GetAgent(a);
                if (!agent.GetLocation().IsPosition())
                    continue;
                if (agent.GetLocation().AsWorldPosition() == pos)
                    symbol = agent.GetSymbol();
            }

            std::cout << symbol;
        }
        std::cout << '\n';
    }

    for (size_t a = 0; a < GetNumAgents(); ++a) {
        const AgentBase& agent = GetAgent(a);
        const auto& st = mAgentState.at(agent.GetID());
        std::cout << "Agent " << agent.GetName() << " | HP: " << st.mHP << "/" << st.mMaxHP << " | ATK: " << st.mAttack
                  << " | HEAL: " << st.mHealCharges << " | SCORE: " << st.mScore << '\n';
    }
    for (const auto& enemy: mEnemies) {
        std::cout << "Enemy#" << enemy.mID << " | HP: " << enemy.mHP << " | Pos: (" << enemy.mPosition.CellX() << ","
                  << enemy.mPosition.CellY() << ")"
                  << " | " << (enemy.mAlive ? "Alive" : "Dead") << '\n';
    }
    std::cout.flush();
}

bool AIWorld::IsWalkable(WorldPosition pos) const {
    if (!main_grid.IsValid(pos))
        return false;
    if (main_grid[pos] == wall_id)
        return false;
    return true;
}

bool AIWorld::IsEnemyAt(WorldPosition pos, size_t* enemy_index) const {
    for (size_t i = 0; i < mEnemies.size(); ++i) {
        if (!mEnemies[i].mAlive)
            continue;
        if (mEnemies[i].mPosition == pos) {
            if (enemy_index)
                *enemy_index = i;
            return true;
        }
    }
    return false;
}

bool AIWorld::IsAgentAt(WorldPosition pos, size_t skip_agent_id) const {
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        const AgentBase& a = GetAgent(i);
        if (a.GetID() == skip_agent_id)
            continue;
        if (!a.GetLocation().IsPosition())
            continue;
        if (a.GetLocation().AsWorldPosition() == pos)
            return true;
    }
    return false;
}

std::optional<size_t> AIWorld::ItemOnFloor(WorldPosition pos) const {
    for (size_t i = 0; i < item_set.size(); ++i) {
        const ItemBase& item = *item_set[i];
        if (!item.GetLocation().IsPosition())
            continue;
        if (item.GetLocation().AsWorldPosition() != pos)
            continue;
        const auto* loot_item = dynamic_cast<const LootItem*>(&item);
        if (!loot_item || loot_item->IsConsumed())
            continue;
        return i;
    }
    return std::nullopt;
}

void AIWorld::HandleEnemyTurn(EnemyState& enemy) {
    if (!enemy.mAlive)
        return;

    size_t target_agent_id = static_cast<size_t>(-1);
    int best_dist = 1'000'000;
    for (size_t i = 0; i < GetNumAgents(); ++i) {
        const AgentBase& a = GetAgent(i);
        if (!a.GetLocation().IsPosition())
            continue;
        const WorldPosition pos = a.GetLocation().AsWorldPosition();
        const int dist =
                static_cast<int>(std::abs(static_cast<int>(enemy.mPosition.CellX()) - static_cast<int>(pos.CellX())) +
                                 std::abs(static_cast<int>(enemy.mPosition.CellY()) - static_cast<int>(pos.CellY())));
        if (dist < best_dist) {
            best_dist = dist;
            target_agent_id = a.GetID();
        }
    }

    if (target_agent_id == static_cast<size_t>(-1))
        return;
    AgentBase& target = GetAgent(target_agent_id);
    if (!target.GetLocation().IsPosition())
        return;

    const WorldPosition target_pos = target.GetLocation().AsWorldPosition();
    const int dist = static_cast<int>(
            std::abs(static_cast<int>(enemy.mPosition.CellX()) - static_cast<int>(target_pos.CellX())) +
            std::abs(static_cast<int>(enemy.mPosition.CellY()) - static_cast<int>(target_pos.CellY())));

    if (dist == 1) {
        AgentState& state = mAgentState[target.GetID()];
        state.mHP -= enemy.mAttack;
        if (mVerboseMode) {
            std::cout << "[Enemy " << enemy.mID << "] attacks " << target.GetName() << " for " << enemy.mAttack
                      << " damage.\n";
        }
        if (state.mHP <= 0) {
            state.mHP = 0;
            mRunOver = true;
            std::cout << target.GetName() << " was slain by enemy " << enemy.mID << ".\n";
        }
        return;
    }

    std::vector<WorldPosition> moves;
    if (target_pos.CellX() > enemy.mPosition.CellX())
        moves.push_back(enemy.mPosition.Right());
    if (target_pos.CellX() < enemy.mPosition.CellX())
        moves.push_back(enemy.mPosition.Left());
    if (target_pos.CellY() > enemy.mPosition.CellY())
        moves.push_back(enemy.mPosition.Down());
    if (target_pos.CellY() < enemy.mPosition.CellY())
        moves.push_back(enemy.mPosition.Up());

    for (const WorldPosition& next: moves) {
        if (!IsWalkable(next))
            continue;
        if (IsEnemyAt(next))
            continue;
        if (IsAgentAt(next, target.GetID()))
            continue;
        enemy.mPosition = next;
        if (mVerboseMode) {
            std::cout << "[Enemy " << enemy.mID << "] moves to (" << enemy.mPosition.CellX() << ","
                      << enemy.mPosition.CellY() << ").\n";
        }
        return;
    }
}

void AIWorld::ConfigAgent(AgentBase& agent) {
    MazeWorld::ConfigAgent(agent);
    agent.AddAction("pickup", PICKUP_ITEM);
    agent.AddAction("use_heal", USE_HEAL);
    agent.AddAction("attack_up", ATTACK_UP);
    agent.AddAction("attack_down", ATTACK_DOWN);
    agent.AddAction("attack_left", ATTACK_LEFT);
    agent.AddAction("attack_right", ATTACK_RIGHT);
    mAgentState[agent.GetID()] = AgentState{};
}

AIWorld::AIWorld() : MazeWorld() {
    auto add_loot = [&](const std::string& name, LootItem::ItemKind kind, int power, WorldPosition pos) {
        auto item = std::make_unique<LootItem>(item_set.size(), name, *this, kind, power);
        item->SetLocation(pos);
        item_set.emplace_back(std::move(item));
    };

    add_loot("Sword", LootItem::ItemKind::Weapon, 1, WorldPosition{3, 5});
    add_loot("Battle Axe", LootItem::ItemKind::Weapon, 2, WorldPosition{18, 2});
    add_loot("Potion", LootItem::ItemKind::Heal, 4, WorldPosition{8, 7});
    add_loot("Med Kit", LootItem::ItemKind::Heal, 6, WorldPosition{13, 8});

    mEnemies.push_back(EnemyState{0, WorldPosition{5, 5}, 5, 1, true});
    mEnemies.push_back(EnemyState{1, WorldPosition{8, 7}, 7, 2, true});
}

std::vector<AIWorld::VisibleItem> AIWorld::GetFloorItems() const {
    std::vector<VisibleItem> visible;
    for (const auto& item_ptr: item_set) {
        const auto* loot_item = dynamic_cast<const LootItem*>(item_ptr.get());
        if (!loot_item || loot_item->IsConsumed())
            continue;
        if (!loot_item->GetLocation().IsPosition())
            continue;
        visible.push_back(VisibleItem{loot_item->GetID(), loot_item->GetLocation().AsWorldPosition(),
                                      loot_item->GetKind(), loot_item->GetPower()});
    }
    return visible;
}

int AIWorld::DoAction(AgentBase& agent, size_t action_id) {
    AgentState& state = mAgentState[agent.GetID()];
    if (state.mHP <= 0)
        return 0;
    const WorldPosition cur_position = agent.GetLocation().AsWorldPosition();

    if (action_id == PICKUP_ITEM) {
        const auto item_id = ItemOnFloor(cur_position);
        if (!item_id.has_value())
            return false;
        auto* loot_item = dynamic_cast<LootItem*>(item_set[*item_id].get());
        if (!loot_item || loot_item->IsConsumed())
            return false;

        if (loot_item->GetKind() == LootItem::ItemKind::Weapon) {
            state.mAttack += loot_item->GetPower();
            state.mScore += 15;
            if (mVerboseMode) {
                std::cout << "[" << agent.GetName() << "] picked up weapon '" << loot_item->GetName() << "' (+"
                          << loot_item->GetPower() << " attack).\n";
            }
        } else {
            state.mHealCharges += loot_item->GetPower();
            state.mScore += 10;
            if (mVerboseMode) {
                std::cout << "[" << agent.GetName() << "] picked up heal item '" << loot_item->GetName() << "' (+"
                          << loot_item->GetPower() << " heal charges).\n";
            }
        }
        loot_item->Consume();
        return true;
    }

    if (action_id == USE_HEAL) {
        if (state.mHealCharges <= 0)
            return false;
        if (state.mHP >= state.mMaxHP)
            return false;
        --state.mHealCharges;
        state.mHP = std::min(state.mMaxHP, state.mHP + 5);
        if (mVerboseMode) {
            std::cout << "[" << agent.GetName() << "] used heal. HP now " << state.mHP << ".\n";
        }
        return true;
    }

    if (action_id >= ATTACK_UP && action_id <= ATTACK_RIGHT) {
        WorldPosition target = cur_position;
        if (action_id == ATTACK_UP)
            target = cur_position.Up();
        if (action_id == ATTACK_DOWN)
            target = cur_position.Down();
        if (action_id == ATTACK_LEFT)
            target = cur_position.Left();
        if (action_id == ATTACK_RIGHT)
            target = cur_position.Right();

        size_t enemy_index = 0;
        if (!IsEnemyAt(target, &enemy_index))
            return false;
        EnemyState& enemy = mEnemies[enemy_index];
        enemy.mHP -= state.mAttack;
        if (mVerboseMode) {
            std::cout << "[" << agent.GetName() << "] attacks Enemy#" << enemy.mID << " for " << state.mAttack
                      << " damage.\n";
        }
        if (enemy.mHP <= 0) {
            enemy.mHP = 0;
            enemy.mAlive = false;
            state.mScore += 50;
            if (mVerboseMode) {
                std::cout << "Enemy#" << enemy.mID << " was defeated.\n";
            }
        } else {
            state.mScore += 5;
        }
        return true;
    }

    WorldPosition new_position = cur_position;
    switch (action_id) {
        case 1:
            new_position = cur_position.Up();
            break;
        case 2:
            new_position = cur_position.Down();
            break;
        case 3:
            new_position = cur_position.Left();
            break;
        case 4:
            new_position = cur_position.Right();
            break;
        default:
            break;
    }
    if (action_id >= 1 && action_id <= 4) {
        if (IsEnemyAt(new_position))
            return false;
        if (IsAgentAt(new_position, agent.GetID()))
            return false;
    }
    return MazeWorld::DoAction(agent, action_id);
}

void AIWorld::RunAgents() {
    for (const auto& agent_ptr: agent_set) {
        AgentBase& agent = *agent_ptr;
        AgentState& state = mAgentState[agent.GetID()];
        if (state.mHP <= 0)
            continue;

        const size_t action_id = agent.SelectAction(main_grid);
        const int result = DoAction(agent, action_id);
        agent.SetActionResult(result);

        if (mVerboseMode) {
            std::cout << "[Turn " << mCurrentTurn << "] " << agent.GetName() << " -> " << ActionName(action_id) << " ("
                      << (result ? "success" : "fail") << ")\n";
        }
        PrintWorldState("After agent action");
        if (!WaitForStep()) {
            mRunOver = true;
            return;
        }
    }
}

void AIWorld::UpdateWorld() {
    ++mCurrentTurn;
    for (auto& enemy: mEnemies)
        HandleEnemyTurn(enemy);

    const bool any_agent_alive =
            std::any_of(mAgentState.begin(), mAgentState.end(), [](const auto& entry) { return entry.second.mHP > 0; });
    const bool any_enemy_alive =
            std::any_of(mEnemies.begin(), mEnemies.end(), [](const EnemyState& enemy) { return enemy.mAlive; });

    if (!any_enemy_alive) {
        std::cout << "All enemies defeated! Agents win.\n";
        mRunOver = true;
    } else if (!any_agent_alive) {
        std::cout << "All agents are down. Enemies win.\n";
        mRunOver = true;
    } else if (mCurrentTurn >= mMaxTurns) {
        std::cout << "Turn limit reached.\n";
        mRunOver = true;
    }

    PrintWorldState("End of world update");
}

void AIWorld::Run() {
    mRunOver = false;
    mCurrentTurn = 0;
    PrintWorldState("Initial State");
    while (!mRunOver) {
        RunAgents();
        if (mRunOver)
            break;
        UpdateWorld();
        if (!WaitForStep()) {
            mRunOver = true;
            break;
        }
    }
}

} // namespace cse498
