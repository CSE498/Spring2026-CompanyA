/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Implementation of the Game class.
 */

#include "Game.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <array>
#include "../../../core/AgentBase.hpp"
// Group 17 AI-agent integration: required so this TU can construct
// SmartEnemyAgent (dungeon goblin) and LearningExplorerAgent (overworld explorer).
// OverWorld.hpp already includes LearningExplorerAgent.hpp for its spawner, but we
// keep it here too for clarity / symmetry with the dungeon spawn path.
#include "../../../Agents/AI/SmartEnemyAgent.hpp"
#include "../../../Agents/AI/LearningExplorerAgent.hpp"
#include "../../../Agents/Classic/TradeSystem/TradeTypes.hpp"
#include "../../../Agents/AI/EnemyAgent.hpp"
#include "../../../Agents/AI/TrailblazerAgent.hpp"
#include "../../../Agents/AI/FetchAgent.hpp"

namespace cse498
{

    constexpr int TURN_DELAY = 100;
    constexpr int TILE_SIZE = 64;
    constexpr int ICON_SIZE = 48;
    constexpr int PICKUP_MESSAGE_DURATION_MS = 1000;
    constexpr int DUNGEON_SPAWN_X = 1;
    constexpr int DUNGEON_SPAWN_Y = 1;

    /// Autonomous overworld agent step delay. Dungeon agents remain turn-driven.
    constexpr Uint32 OVERWORLD_AGENT_STEP_DELAY = 900;

    // -----------------------------------------------------------------------
    //  Initialization
    // -----------------------------------------------------------------------

    bool Game::Initialize()
    {
        if (!mGameView->Initialize())
        {
            std::cerr << "GameView initialization failed." << std::endl;
            return false;
        }

        std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
        std::cout << "Asset Dir: " << std::string(ASSETS_DIR) << std::endl;

        SDL_Renderer *renderer = mGameView->GetRenderer();

        // Title text
        mTitleText.SetRenderer(renderer);
        mTitleText.SetContent("Slay the Dungeon");
        mTitleText.SetSize(ICON_SIZE);
        mTitleText.SetBold(true);

        // Pause text
        mPauseText.SetRenderer(renderer);
        mPauseText.SetContent("Paused");
        mPauseText.SetSize(ICON_SIZE);
        mPauseText.SetBold(true);

        // Item pickup notifications
        mPickupText.SetRenderer(renderer);
        mPickupText.SetSize(20);

        // Stats text
        mStatsText.SetRenderer(renderer);

        // Set up image manager and load all tile assets
        mImageManager = std::make_unique<ImageManager>(renderer);

        // Helper lambda to load and propagate errors
        auto LoadCheck = [&](const std::string &name, const std::string &path) -> bool
        {
            auto result = mImageManager->LoadImage(name, path);
            if (!result)
            {
                std::cerr << "LoadImage failed: " << result.error() << std::endl;
                return false;
            }
            return true;
        };

        // Overworld tiles
        if (!LoadCheck("ow_grass", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/grass.png"))
            return false;
        if (!LoadCheck("ow_grass_flowers", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/grass_detail.png"))
            return false;
        if (!LoadCheck("ow_grass_bones", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/grass_detail.png"))
            return false;
        if (!LoadCheck("ow_grass_mud", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/path.png"))
            return false;
        if (!LoadCheck("ow_grass_rock", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/grass.png"))
            return false;
        if (!LoadCheck("ow_entrance", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/entrance.png"))
            return false;
        if (!LoadCheck("ow_wall_left", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/wall.png"))
            return false;
        if (!LoadCheck("ow_wall_right", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/wall.png"))
            return false;
        if (!LoadCheck("ow_wall_top", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/wall.png"))
            return false;
        if (!LoadCheck("ow_wall_bottom", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/wall.png"))
            return false;
        if (!LoadCheck("ow_wall_corner", std::string(ASSETS_DIR) + "/" + "interactive_world/terrain/wall.png"))
            return false;
        if (!LoadCheck("ow_building", std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/lumber_yard.png"))
            return false;
        if (!LoadCheck("ow_building_lumberyard",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/lumber_yard.png"))
            return false;
        if (!LoadCheck("ow_building_lumberyard_upgraded",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/lumber_yard_upgraded.png"))
            return false;
        if (!LoadCheck("ow_building_quarry", std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/quarry.png"))
            return false;
        if (!LoadCheck("ow_building_quarry_upgraded",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/quarry_upgraded.png"))
            return false;
        if (!LoadCheck("ow_building_mine", std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/mine.png"))
            return false;
        if (!LoadCheck("ow_building_mine_upgraded",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/mine_upgraded.png"))
            return false;
        if (!LoadCheck("ow_town_hall", std::string(ASSETS_DIR) + "/" + "interactive_world/buildings/town_hall.png"))
            return false;
        if (!LoadCheck("resource_wood_spawn", std::string(ASSETS_DIR) + "/" + "interactive_world/resources/wood_spawn.png"))
            return false;
        if (!LoadCheck("resource_wood_spawn_empty",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/wood_spawn_empty.png"))
            return false;
        if (!LoadCheck("resource_wood_spawn_full",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/wood_spawn_full.png"))
            return false;
        if (!LoadCheck("resource_stone_spawn",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/stone_spawn.png"))
            return false;
        if (!LoadCheck("resource_stone_spawn_empty",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/stone_spawn_empty.png"))
            return false;
        if (!LoadCheck("resource_stone_spawn_full",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/stone_spawn_full.png"))
            return false;
        if (!LoadCheck("resource_metal_spawn",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/metal_spawn.png"))
            return false;
        if (!LoadCheck("resource_metal_spawn_empty",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/metal_spawn_empty.png"))
            return false;
        if (!LoadCheck("resource_metal_spawn_full",
                    std::string(ASSETS_DIR) + "/" + "interactive_world/resources/metal_spawn_full.png"))
            return false;
        if (!LoadCheck("resource_manager", std::string(ASSETS_DIR) + "/" + "interactive_world/agents/resource_manager.png"))
            return false;
        if (!LoadCheck("fetch_agent", std::string(ASSETS_DIR) + "/" + "interactive_world/agents/fetch_agent.png"))
            return false;
        if (!LoadCheck("interactive_player", std::string(ASSETS_DIR) + "/" + "interactive_world/agents/player.png"))
            return false;


        // Mobs
        if (!LoadCheck("skeleton", std::string(ASSETS_DIR) + "/" +  "agents/monsters/agent_monster_skeleton.png"))
            return false;
        if (!LoadCheck("goblin", std::string(ASSETS_DIR) + "/" + "agents/monsters/agent_monster_goblin.png")) return false;
        if (!LoadCheck("dun_monster", std::string(ASSETS_DIR) + "/" + "agents/monsters/agent_monster_skeleton.png")) return false;

        // --- Level 1 floors (forest) ---
        if (!LoadCheck("floor_l1v1", std::string(ASSETS_DIR) + "/" +  "world/forest/floor_tiles/tile_grass_1.png")) return false;
        if (!LoadCheck("floor_l1v2", std::string(ASSETS_DIR) + "/" +  "world/forest/floor_tiles/tile_grass_2.png")) return false;
        if (!LoadCheck("floor_l1v3", std::string(ASSETS_DIR) + "/" +  "world/forest/floor_tiles/tile_grass_3.png")) return false;
        if (!LoadCheck("floor_l1v4", std::string(ASSETS_DIR) + "/" +  "world/forest/floor_tiles/tile_grass_4.png")) return false;
        if (!LoadCheck("floor_l1v5", std::string(ASSETS_DIR) + "/" +  "world/forest/floor_tiles/tile_grass_5.png")) return false;
        // --- Level 2 floors (cave) ---
        if (!LoadCheck("floor_l2v1", std::string(ASSETS_DIR) + "/" +  "world/cave/floor_tiles/tile_cave_1.png")) return false;
        if (!LoadCheck("floor_l2v2", std::string(ASSETS_DIR) + "/" +  "world/cave/floor_tiles/tile_cave_2.png")) return false;
        if (!LoadCheck("floor_l2v3", std::string(ASSETS_DIR) + "/" +  "world/cave/floor_tiles/tile_cave_3.png")) return false;
        if (!LoadCheck("floor_l2v4", std::string(ASSETS_DIR) + "/" +  "world/cave/floor_tiles/tile_cave_4.png")) return false;
        if (!LoadCheck("floor_l2v5", std::string(ASSETS_DIR) + "/" +  "world/cave/floor_tiles/tile_cave_5.png")) return false;
        // --- Level 3 floors (dungeon) ---
        if (!LoadCheck("floor_l3v1", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_1.png")) return false;
        if (!LoadCheck("floor_l3v2", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_2.png")) return false;
        if (!LoadCheck("floor_l3v3", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_3.png")) return false;
        if (!LoadCheck("floor_l3v4", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_4.png")) return false;
        if (!LoadCheck("floor_l3v5", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_5.png")) return false;
        // --- Level 4 floors (castle) ---
        if (!LoadCheck("floor_l4v1", std::string(ASSETS_DIR) + "/" +  "world/castle/floor_tiles/tile_wood_1.png")) return false;
        if (!LoadCheck("floor_l4v2", std::string(ASSETS_DIR) + "/" +  "world/castle/floor_tiles/tile_wood_2.png")) return false;
        if (!LoadCheck("floor_l4v3", std::string(ASSETS_DIR) + "/" +  "world/castle/floor_tiles/tile_wood_3.png")) return false;
        if (!LoadCheck("floor_l4v4", std::string(ASSETS_DIR) + "/" +  "world/castle/floor_tiles/tile_wood_4.png")) return false;
        if (!LoadCheck("floor_l4v5", std::string(ASSETS_DIR) + "/" +  "world/castle/floor_tiles/tile_wood_5.png")) return false;

        // --- Generic wall (#) ---
        if (!LoadCheck("wall", std::string(ASSETS_DIR) + "/" +  "gui/black_tile.png")) return false;

        // --- Level 1 walls (forest) ---
        if (!LoadCheck("wall_l1v1", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v2", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_bottom_forest.png")) return false;
        if (!LoadCheck("wall_l1v13", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_left_forest.png")) return false;
        if (!LoadCheck("wall_l1v4", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_right_forest.png")) return false;
        if (!LoadCheck("wall_l1v5", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v6", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v7", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/door_left_forest.png")) return false;
        if (!LoadCheck("wall_l1v8", std::string(ASSETS_DIR) + "/" +  "world/forest/walls/external/door_right_forest.png")) return false;
        // --- Level 2 walls (cave) ---
        if (!LoadCheck("wall_l2v1", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v2", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_bottom_cave.png")) return false;
        if (!LoadCheck("wall_l2v3", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_left_cave.png")) return false;
        if (!LoadCheck("wall_l2v4", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_right_cave.png")) return false;
        if (!LoadCheck("wall_l2v5", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v6", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v7", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/door_left_cave.png")) return false;
        if (!LoadCheck("wall_l2v8", std::string(ASSETS_DIR) + "/" +  "world/cave/walls/external/door_right_cave.png")) return false;
        // --- Level 3 walls (dungeon) ---
        if (!LoadCheck("wall_l3v1", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v2", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_bottom_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v3", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_left_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v4", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_right_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v5", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v6", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v7", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/door_left_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v8", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/door_right_dungeon.png")) return false;
        // --- Level 4 walls (castle) ---
        if (!LoadCheck("wall_l4v1", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v2", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_bottom_castle.png")) return false;
        if (!LoadCheck("wall_l4v3", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_left_castle.png")) return false;
        if (!LoadCheck("wall_l4v4", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_right_castle.png")) return false;
        if (!LoadCheck("wall_l4v5", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v6", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v7", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/door_left_castle.png")) return false;
        if (!LoadCheck("wall_l4v8", std::string(ASSETS_DIR) + "/" +  "world/castle/walls/external/door_right_castle.png")) return false;

        // --- Shared special tiles ---
        if (!LoadCheck("wall_trap", std::string(ASSETS_DIR) + "/" +  "world/dungeon/floor_tiles/tile_stoneBrick_3.png")) return false;
        if (!LoadCheck("wall_loot", std::string(ASSETS_DIR) + "/" +  "tiles/item_spawn.png")) return false;
        if (!LoadCheck("wall_skeleton", std::string(ASSETS_DIR) + "/" +  "agents/monsters/agent_monster_skeleton.png")) return false;
        if (!LoadCheck("wall_goblin", std::string(ASSETS_DIR) + "/" +  "agents/monsters/agent_monster_goblin.png")) return false;
        // KAREN: Changes to match DungeonWorld
        // if (!LoadCheck("wall_secret", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/door_right_dungeon.png")) return false;
        // if (!LoadCheck("exit", std::string(ASSETS_DIR) + "/" +  "world/dungeon/walls/external/door_left_dungeon.png")) return false;
        // Secret doors
        if (!LoadCheck("wall_secret_top",    std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_right_dungeon.png")) return false;
        if (!LoadCheck("wall_secret_bottom", std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_right_dungeon.png")) return false;
        if (!LoadCheck("wall_secret_left",   std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_right_dungeon.png")) return false;
        if (!LoadCheck("wall_secret_right",  std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_right_dungeon.png")) return false;
        // Exits
        if (!LoadCheck("exit_l1", std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_left_dungeon.png")) return false;
        if (!LoadCheck("exit_l2", std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_left_dungeon.png")) return false;
        if (!LoadCheck("exit_l3", std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_left_dungeon.png")) return false;
        if (!LoadCheck("exit_l4", std::string(ASSETS_DIR) + "/world/dungeon/walls/external/door_left_dungeon.png")) return false;

        // Item sprites — keyed by item name to match what Inventory stores
        if (!LoadCheck("Sword", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +1", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +2", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +3", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +4", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +5", std::string(ASSETS_DIR) + "/" +  "items/item_sword_1.png")) return false;

        if (!LoadCheck("Bow", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +1", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +2", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +3", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +4", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +5", std::string(ASSETS_DIR) + "/" +  "items/item_bow_1.png")) return false;

        if (!LoadCheck("Healing Potion", std::string(ASSETS_DIR) + "/" +  "items/item_potion_healing.png")) return false;
        if (!LoadCheck("Defense Potion", std::string(ASSETS_DIR) + "/" +  "items/item_potion_defense.png")) return false;
        if (!LoadCheck("Speed Potion", std::string(ASSETS_DIR) + "/" +  "items/item_potion_speed.png")) return false;

        if (!LoadCheck("Axe", std::string(ASSETS_DIR) + "/" +  "items/item_axe.png")) return false;
        if (!LoadCheck("Pickaxe", std::string(ASSETS_DIR) + "/" +  "items/item_pickaxe.png")) return false;
        if (!LoadCheck("Shovel", std::string(ASSETS_DIR) + "/" +  "items/item_shovel.png")) return false;

        // Player
        if (!LoadCheck("player", std::string(ASSETS_DIR) + "/" +  "agents/playerCharacter/agent_player.png"))
            return false;

        // Merchant Items
        if (!LoadCheck("Crown", std::string(ASSETS_DIR) + "/" + "items/item_crown.png")) return false;

        // UI
        if (!LoadCheck("inventory_bar", std::string(ASSETS_DIR) + "/" +  "/gui/inventory_bar.png"))
            return false;

        std::cout << "Asset loads complete." << std::endl;

        // Analytics setup
        mAnalyticsManager = std::make_shared<AnalyticsManager>();
        mStatsTracker = std::make_unique<StatsTracker>();

        // World Setups
        SetupOverworld();
        SetupDungeon();

        SetupMainMenu();
        SetupPauseMenu();
        return true;
    }

    void BuildImageGridFromWorldGrid(const WorldGrid& worldGrid, ImageGrid& imageGrid)
    {
    const size_t world_w = worldGrid.GetWidth();
    const size_t world_h = worldGrid.GetHeight();

    for (size_t y = 0; y < world_h; ++y)
    {
        for (size_t x = 0; x < world_w; ++x)
        {
            WorldPosition pos(x, y);
            const std::string& cell_name = worldGrid.GetCellTypeName(worldGrid[pos]);
            imageGrid.SetCell(x, y, cell_name);
        }
    }
    }

    void Game::SetupOverworld() {
        mOverWorld = std::make_shared<InteractiveWorld>();

        mOverWorld->GetInventory().AddItem(ItemType::Wood, 10);
        mOverWorld->GetInventory().AddItem(ItemType::Stone, 5);

        auto townHallPtr = std::make_unique<TownHall>(mOverWorld->GetNextAgentId(), "Town Hall", *mOverWorld,
                                                    mOverWorld->GetInventoryPtr());
        townHallPtr->SetSymbol('T');
        TownHall& townHall = mOverWorld->AddAgent(std::move(townHallPtr));
        mOverWorld->AddTownHall(townHall, WorldPosition{8, 6});

        Building& lumberYardRef = mOverWorld->AddAgent<Building>("Lumber Yard");
        lumberYardRef.SetSymbol('L');
        lumberYardRef.AddUpgrade(ItemType::Wood, 15);

        Building& quarryRef = mOverWorld->AddAgent<Building>("Quarry");
        quarryRef.SetSymbol('Q');
        quarryRef.AddUpgrade(ItemType::Wood, 50);
        quarryRef.AddUpgrade(ItemType::Stone, 50);
        quarryRef.AddUpgrade(ItemType::Metal, 35);

        Building& mineRef = mOverWorld->AddAgent<Building>("Mine");
        mineRef.SetSymbol('M');
        mineRef.AddUpgrade(ItemType::Stone, 100);
        mineRef.AddUpgrade(ItemType::Metal, 50);
        mineRef.AddUpgrade(ItemType::Metal, 100);

        auto woodSpawnPtr =
                std::make_unique<ResourceSpawn>(mOverWorld->GetNextAgentId(), "Wood Spawn", *mOverWorld, ItemType::Wood);
        woodSpawnPtr->SetSymbol('l');
        ResourceSpawn& woodSpawnRef = mOverWorld->AddAgent(std::move(woodSpawnPtr));
        woodSpawnRef.SetMaxCollectionQuantity(10);
        mOverWorld->AddResourceSpawn(woodSpawnRef, WorldPosition{15, 1});

        auto stoneSpawnPtr =
                std::make_unique<ResourceSpawn>(mOverWorld->GetNextAgentId(), "Stone Spawn", *mOverWorld, ItemType::Stone);
        stoneSpawnPtr->SetSymbol('q');
        ResourceSpawn& stoneSpawnRef = mOverWorld->AddAgent(std::move(stoneSpawnPtr));
        stoneSpawnRef.SetMaxCollectionQuantity(5);
        stoneSpawnRef.SetLocation(WorldPosition{1, 11});

        auto metalSpawnPtr =
                std::make_unique<ResourceSpawn>(mOverWorld->GetNextAgentId(), "Metal Spawn", *mOverWorld, ItemType::Metal);
        metalSpawnPtr->SetSymbol('m');
        ResourceSpawn& metalSpawnRef = mOverWorld->AddAgent(std::move(metalSpawnPtr));
        metalSpawnRef.SetMaxCollectionQuantity(5);
        metalSpawnRef.SetLocation(WorldPosition{15, 11});

        // Wood starts unlocked and produces 10 wood every 5 seconds.
        mOverWorld->AddProducer(std::make_shared<ResourceProducer>(lumberYardRef, woodSpawnRef, ItemType::Wood, 2));

        mOverWorld->AddBuilding(lumberYardRef, WorldPosition{12, 3});
        quarryRef.SetLocation(WorldPosition{4, 9});
        mineRef.SetLocation(WorldPosition{12, 9});

        auto configureFetcher = [](FetchAgent& fetcher, AgentBase& origin, AgentBase& deposit, ItemType itemType,
                                char symbol, WorldPosition position) {
            fetcher.SetOrigin(origin).SetDepositPoint(deposit).SetItemType(itemType).SetSymbol(symbol).SetLocation(
                    position);
        };

        FetchAgent& woodToLumberYard = mOverWorld->AddAgent<FetchAgent>("Wood To Lumber Yard");
        configureFetcher(woodToLumberYard, woodSpawnRef, lumberYardRef, ItemType::Wood, '1', WorldPosition{14, 2});

        FetchAgent& woodToTownHall = mOverWorld->AddAgent<FetchAgent>("Lumber Yard To Town Hall");
        configureFetcher(woodToTownHall, lumberYardRef, townHall, ItemType::Wood, '2', WorldPosition{12, 4});

        FetchAgent& stoneToQuarry = mOverWorld->AddAgent<FetchAgent>("Stone To Quarry");
        stoneToQuarry.SetOrigin(stoneSpawnRef)
                .SetDepositPoint(quarryRef)
                .SetItemType(ItemType::Stone)
                .SetSymbol('3')
                .SetLocation(WorldPosition{2, 10});

        FetchAgent& stoneToTownHall = mOverWorld->AddAgent<FetchAgent>("Quarry To Town Hall");
        stoneToTownHall.SetOrigin(quarryRef)
                .SetDepositPoint(townHall)
                .SetItemType(ItemType::Stone)
                .SetSymbol('4')
                .SetLocation(WorldPosition{5, 8});

        FetchAgent& metalToMine = mOverWorld->AddAgent<FetchAgent>("Metal To Mine");
        metalToMine.SetOrigin(metalSpawnRef)
                .SetDepositPoint(mineRef)
                .SetItemType(ItemType::Metal)
                .SetSymbol('5')
                .SetLocation(WorldPosition{14, 10});

        FetchAgent& metalToTownHall = mOverWorld->AddAgent<FetchAgent>("Mine To Town Hall");
        metalToTownHall.SetOrigin(mineRef)
                .SetDepositPoint(townHall)
                .SetItemType(ItemType::Metal)
                .SetSymbol('6')
                .SetLocation(WorldPosition{12, 8});

        ResourceManagementAgent& resourceManager = mOverWorld->AddAgent<ResourceManagementAgent>("Resource Manager");
        resourceManager.SetInventory(mOverWorld->GetInventoryPtr()).SetSymbol('7');
        resourceManager.AddManagedBuilding(lumberYardRef, true);
        resourceManager.AddManagedBuilding(quarryRef, false);
        resourceManager.AddManagedBuilding(mineRef, false);
        resourceManager.SetLocation(WorldPosition{2, 3});

        woodToLumberYard.SetActive(true);
        woodToTownHall.SetActive(true);
        stoneToQuarry.SetActive(false);
        stoneToTownHall.SetActive(false);
        metalToMine.SetActive(false);
        metalToTownHall.SetActive(false);

        // Locked lanes keep valid future positions for AI compatibility but
        // are hidden/inactive until their hire callback places blockers and
        // starts their producers.
        resourceManager.AddHireableLane("Quarry Lane", stoneToQuarry, stoneToTownHall, quarryRef, 10,
                                        [this, stoneSpawn = &stoneSpawnRef, quarry = &quarryRef,
                                        firstHauler = &stoneToQuarry, secondHauler = &stoneToTownHall]() {
                                            mOverWorld->AddResourceSpawn(*stoneSpawn, WorldPosition{1, 11});
                                            mOverWorld->AddBuilding(*quarry, WorldPosition{4, 9});
                                            // Stone produces 5 stone every 10 seconds after the lane unlocks.
                                            mOverWorld->AddProducer(std::make_shared<ResourceProducer>(
                                                    *quarry, *stoneSpawn, ItemType::Stone, 0.5f, std::chrono::seconds(10)));
                                            firstHauler->SetLocation(WorldPosition{2, 10});
                                            secondHauler->SetLocation(WorldPosition{5, 8});
                                        });
        resourceManager.AddHireableLane(
                "Mine Lane", metalToMine, metalToTownHall, mineRef, 20,
                [this, metalSpawn = &metalSpawnRef, mine = &mineRef, firstHauler = &metalToMine,
                secondHauler = &metalToTownHall]() {
                    mOverWorld->AddResourceSpawn(*metalSpawn, WorldPosition{15, 11});
                    mOverWorld->AddBuilding(*mine, WorldPosition{12, 9});
                    // Metal produces 5 metal every 15 seconds after the lane unlocks.
                    mOverWorld->AddProducer(std::make_shared<ResourceProducer>(*mine, *metalSpawn, ItemType::Metal,
                                                                            1.0f / 3.0f, std::chrono::seconds(15)));
                    firstHauler->SetLocation(WorldPosition{14, 10});
                    secondHauler->SetLocation(WorldPosition{12, 8});
                });

        PlayerAgent& player = *mOverWorld->GetPlayer();
        player.AddGold(10000); // starting gold for trading demo
        mOverworldPlayer = &player;

        // Farming merchant NPC
        // auto& farmer = mOverWorld->AddAgent<FarmingAgent>("Farmer");
        // farmer.SetLocation(WorldPosition{4, 6});
        // farmer.SetHomePosition(WorldPosition{4, 6});
        // farmer.SetAssignedBuilding(&lumberYardRef);
        // farmer.SetWorkInterval(20);
        // farmer.SetRestockAmount(3);
        // farmer.SetRestockItemName("wheat");

        // // Set up the farmer's shop offers
        // farmer.AddInitialOffer(TradeOffer{
        //     .mItemName = "wheat",
        //     .mBuyPrice = 2,
        //     .mSellPrice = 1,
        //     .mItemValue = 2,
        //     .mStockMode = TradeStockMode::Limited,
        //     .mStock = 5
        // });
        // farmer.AddInitialOffer(TradeOffer{
        //     .mItemName = "apple",
        //     .mBuyPrice = 3,
        //     .mSellPrice = 1,
        //     .mItemValue = 3,
        //     .mStockMode = TradeStockMode::Unlimited,
        //     .mStock = 0
        // });
        // farmer.SetGold(50);

        // // Resource trader — buys world resources for gold
        // auto& trader = mOverWorld->AddAgent<MerchantAgent>("Trader");
        // trader.SetLocation(WorldPosition{10, 6});
        // trader.SetTradeGreeting("I'll buy your resources.");
        // trader.SetGold(11000); // enough so that if anyone actually tries to buy the crown they can

        // trader.AddInitialOffer(TradeOffer{
        //     .mItemName = "Wood",
        //     .mBuyPrice = 0,
        //     .mSellPrice = 2,
        //     .mItemValue = 2,
        //     .mStockMode = TradeStockMode::Unlimited,
        //     .mStock = 0
        // });
        // trader.AddInitialOffer(TradeOffer{
        //     .mItemName = "Stone",
        //     .mBuyPrice = 0,
        //     .mSellPrice = 3,
        //     .mItemValue = 3,
        //     .mStockMode = TradeStockMode::Unlimited,
        //     .mStock = 0
        // });
        // trader.AddInitialOffer(TradeOffer{
        //     .mItemName = "Metal",
        //     .mBuyPrice = 0,
        //     .mSellPrice = 5,
        //     .mItemValue = 5,
        //     .mStockMode = TradeStockMode::Unlimited,
        //     .mStock = 0
        // });
        // trader.AddInitialOffer(TradeOffer{
        //     .mItemName = "Crown",
        //     .mBuyPrice = 10000,
        //     .mSellPrice = 5000,
        //     .mItemValue = 10000,
        //     .mStockMode = TradeStockMode::Limited,
        //     .mStock = 1
        // });

        // Build the image grid
        const WorldGrid& grid = mOverWorld->GetGrid();
        size_t world_w = grid.GetWidth();
        size_t world_h = grid.GetHeight();

        mOverworldGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);

        for (size_t y = 0; y < world_h; ++y) {
            for (size_t x = 0; x < world_w; ++x) {
                WorldPosition pos(x, y);
                std::string cell_name = grid.GetCellTypeName(grid[pos]);
                if (cell_name == "ow_building") {
                    cell_name = "ow_grass";
                }
                mOverworldGrid->SetCell(x, y, cell_name);
            }
        }

        mOverWorld->SetAnalyticsManager(mAnalyticsManager);
    }



    void Game::SetupDungeon()
    {
        mDungeonWorld = std::make_unique<DungeonWorld>();

        const WorldGrid &grid = mDungeonWorld->GetGrid();
        size_t world_w = grid.GetWidth();
        size_t world_h = grid.GetHeight();

        mDungeonGrid = std::make_unique<ImageGrid>(world_w, world_h, TILE_SIZE, TILE_SIZE);

        // KAREN: DungeonWorld already creates a player in its constructor
        // auto& player = mDungeonWorld->AddAgent<PlayerAgent>("Player");
        // player.SetLocation(WorldPosition{1, 1});
        // mDungeonPlayer = &player;
        mDungeonPlayer = mDungeonWorld->GetPlayer();
        mDungeonPlayer->SetMaxHealth(10);
        mDungeonPlayer->SetHealth(10);

        std::cout << "Dungeon player ID: " << mDungeonPlayer->GetID() << std::endl;

        /*
         * Helper: PlaceOnNextFloor
         * Scans the dungeon grid for the first valid walkable floor tile
         * and assigns that position to the given agent.
         *
         * We skip the player spawn at {1,1} and rely on the convention
         * that all dungeon floor tiles have names starting with "floor".
         * this also tracks used tiles so agents don't all spawn on the
         * same tile.
         */
        std::unordered_set<std::string> usedPositions;
        [[maybe_unused]]
        auto PlaceOnNextFloor = [&](AgentBase& agent) {
            for (size_t y = 0; y < world_h; ++y) {
                for (size_t x = 0; x < world_w; ++x) {
                    if (x == 1 && y == 1) continue;

                    WorldPosition pos(x, y);
                    std::string key = std::to_string(x) + "," + std::to_string(y);

                    if (usedPositions.count(key)) continue;

                    const std::string& cell_name = grid.GetCellTypeName(grid[pos]);

                    if (cell_name.rfind("floor", 0) == 0) {
                        agent.SetLocation(pos);
                        usedPositions.insert(key);
                        return;
                    }
                }
            }
        };

        // KAREN: DungeonWorld works with Enemy, but not EnemyAgent, so this should have
        // been placed as a Group 15 hook instead. 
        /// @internal Group 17 AI hook: drop a @ref SmartEnemyAgent into the dungeon.
        ///
        /// The current @c DungeonWorld API does not expose a "first room center"
        /// helper, so we scan the grid for the first cell whose registered type
        /// name starts with "floor" (all dungeon floor tiles use that convention)
        /// and is distinct from the player's spawn at {1,1}. This keeps the goblin
        /// on a walkable tile without assuming anything about the dungeon layout.
        // auto& goblin = mDungeonWorld->AddAgent<SmartEnemyAgent>("Goblin");
        // PlaceOnNextFloor(goblin);

        // auto& enemy = mDungeonWorld->AddAgent<EnemyAgent>("EnemyAgent");
        // PlaceOnNextFloor(enemy);

        // auto& smartEnemy = mDungeonWorld->AddAgent<SmartEnemyAgent>("SmartEnemy");
        // PlaceOnNextFloor(smartEnemy);

        // auto& trailblazer = mDungeonWorld->AddAgent<TrailblazerAgent>("Trailblazer");
        // PlaceOnNextFloor(trailblazer);
        // std::cout << "Trailblazer at: " << trailblazer.GetLocation().AsWorldPosition().CellX()
        //   << "," << trailblazer.GetLocation().AsWorldPosition().CellY() << std::endl;
        
        RebuildDungeonGrid();
        mDungeonWorld->SetAnalyticsManager(mAnalyticsManager);
    }

    void Game::RebuildDungeonGrid() {
        const WorldGrid& grid = mDungeonWorld->GetGrid();
        size_t world_w = grid.GetWidth();
        size_t world_h = grid.GetHeight();

        mDungeonGrid = std::make_unique<ImageGrid>(world_w, world_h, TILE_SIZE, TILE_SIZE);

        BuildImageGridFromWorldGrid(grid, *mDungeonGrid);
    }

    void Game::SetupMainMenu()
    {
        mMainMenu.Clear();

        mMainMenu.AddOption("Enter Game", [this]() { TransitionTo(GameState::OVERWORLD); });

        mMainMenu.AddOption("Controls", [this]() { TransitionTo(GameState::CONTROLS); });

        mMainMenu.AddOption("Quit", [this]() { Quit(); });
    }

    void Game::SetupPauseMenu()
    {
        mPauseMenu.Clear();

        mPauseMenu.AddOption("Resume", [this]() { Resume(); });

        mPauseMenu.AddOption("Go to Dungeon World",
                             [this]()
                             {
                                 TransitionTo(GameState::DUNGEON);
                                 mPreviousState = GameState::DUNGEON;
                             });

        mPauseMenu.AddOption("Go to Overworld",
                             [this]()
                             {
                                 TransitionTo(GameState::OVERWORLD);
                                 mPreviousState = GameState::OVERWORLD;
                             });

        mPauseMenu.AddOption("Stats", [this]() {
            if (mAnalyticsManager) {
                // Sync action logs then clear so they don't re-count next visit
                mOverWorld->SyncAgentLogsToAnalytics();
                mDungeonWorld->SyncAgentLogsToAnalytics();

                for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i)
                    mOverWorld->GetAgentByIndex(i).GetActionLog().Clear();
                for (size_t i = 0; i < mDungeonWorld->GetNumAgents(); ++i)
                    mDungeonWorld->GetAgentByIndex(i).GetActionLog().Clear();

                // Flush combat stats
                const auto& runStats = mAnalyticsManager->GetCurrentRunStats();
                if (runStats.damageDealt > 0 || runStats.enemiesKilled > 0) {
                    mAnalyticsManager->LogDamageDealt(runStats.damageDealt);
                    mAnalyticsManager->LogEnemiesKilled(runStats.enemiesKilled);
                    mAnalyticsManager->ResetCurrentRunStats();
                }

                mDashboardSnapshot = mStatsTracker->BuildSnapshot(*mAnalyticsManager);
            }
            TransitionTo(GameState::STATS);
        });

        mPauseMenu.AddOption("Controls", [this]() { TransitionTo(GameState::CONTROLS); });

        mPauseMenu.AddOption("Quit to Main Menu", [this]() { TransitionTo(GameState::MAIN_MENU); });
    }

    // -----------------------------------------------------------------------
    //  Main Loop
    // -----------------------------------------------------------------------

    void Game::Run()
    {
        mRunning = true;
        while (mRunning && mState != GameState::QUIT)
        {
            HandleEvents();

            switch (mState)
            {
            case GameState::MAIN_MENU:
                UpdateMainMenu();
                break;
            case GameState::OVERWORLD:
                UpdateOverworld();
                break;
            case GameState::DUNGEON:
                UpdateDungeon();
                break;
            case GameState::PAUSED:
                UpdatePaused();
                break;
            case GameState::STATS:
                UpdateStats();
                break;
            case GameState::CONTROLS:
                UpdateControls();
                break;
            case GameState::TRADING:
                UpdateTrading();
                break;
            case GameState::RESOURCE_MANAGEMENT:
                UpdateResourceManagement();
                break;
            default:
                break;
            }

            mGameView->Clear();
            switch (mState)
            {
            case GameState::MAIN_MENU:
                RenderMainMenu();
                break;
            case GameState::OVERWORLD:
                RenderOverworld();
                break;
            case GameState::DUNGEON:
                RenderDungeon();
                break;
            case GameState::PAUSED:
                RenderPaused();
                break;
            case GameState::STATS:
                RenderStats();
                break;
            case GameState::CONTROLS:
                RenderControls();
                break;
            case GameState::TRADING:
                RenderTrading();
                break;
            case GameState::RESOURCE_MANAGEMENT:
                RenderResourceManagement();
                break;
            default:
                break;
            }
            mGameView->Present();
        }

        mGameView->Shutdown();
    }

    // -----------------------------------------------------------------------
    //  Event Handling
    // -----------------------------------------------------------------------

    void Game::HandleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                mRunning = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {

                    // Navigation in menus
                    case SDLK_UP:
                        if (mState == GameState::MAIN_MENU)
                            mMainMenu.SelectPrevious();
                        if (mState == GameState::PAUSED)
                            mPauseMenu.SelectPrevious();
                        if (mState == GameState::TRADING && mActiveMerchant) {
                            int count = 0;
                            if (mTradeBuyMode) {
                                for (const auto& offer : mActiveMerchant->GetOffers()) {
                                    if (offer.mBuyPrice > 0) count++;
                                }
                            } else {
                                count = static_cast<int>(mActiveMerchant->GetOffers().size());
                            }
                            if (count > 0) mTradeMenuSelection = (mTradeMenuSelection - 1 + count) % count;
                        }
                        if (mState == GameState::RESOURCE_MANAGEMENT && mActiveResourceManager) {
                            int count = 0;
                            if (mResourceMenuTab == 0) {
                                count = static_cast<int>(mOverWorld->GetBuildings().size());
                            } else if (mResourceMenuTab == 1) {
                                count = static_cast<int>(mActiveResourceManager->GetHireableLaneCount());
                            } else {
                                count = 3;
                            }
                            if (count > 0)
                                mResourceMenuSelection = (mResourceMenuSelection - 1 + count) % count;
                        }
                        break;
                    case SDLK_DOWN:
                        if (mState == GameState::MAIN_MENU)
                            mMainMenu.SelectNext();
                        if (mState == GameState::PAUSED)
                            mPauseMenu.SelectNext();
                        if (mState == GameState::TRADING && mActiveMerchant) {
                            int count = 0;
                            if (mTradeBuyMode) {
                                for (const auto& offer : mActiveMerchant->GetOffers()) {
                                    if (offer.mBuyPrice > 0) count++;
                                }
                            } else {
                                count = static_cast<int>(mActiveMerchant->GetOffers().size());
                            }
                            if (count > 0) mTradeMenuSelection = (mTradeMenuSelection + 1 + count) % count;
                        }
                        if (mState == GameState::RESOURCE_MANAGEMENT && mActiveResourceManager) {
                            int count = 0;
                            if (mResourceMenuTab == 0) {
                                count = static_cast<int>(mOverWorld->GetBuildings().size());
                            } else if (mResourceMenuTab == 1) {
                                count = static_cast<int>(mActiveResourceManager->GetHireableLaneCount());
                            } else {
                                count = 3;
                            }
                            if (count > 0)
                                mResourceMenuSelection = (mResourceMenuSelection + 1) % count;
                        }
                        break;

                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        if (mState == GameState::TRADING) {
                            mTradeBuyMode = !mTradeBuyMode;
                            mTradeMenuSelection = 0;
                        }
                        if (mState == GameState::RESOURCE_MANAGEMENT) {
                            const int direction = (event.key.keysym.sym == SDLK_RIGHT) ? 1 : 2;
                            mResourceMenuTab = (mResourceMenuTab + direction) % 3;
                            mResourceMenuSelection = 0;
                        }
                        break;
                    case SDLK_RETURN:
                        if (mState == GameState::MAIN_MENU)
                            mMainMenu.ActivateSelected();
                        if (mState == GameState::PAUSED)
                            mPauseMenu.ActivateSelected();
                        if (mState == GameState::TRADING && mActiveMerchant) {
                            if (mTradeBuyMode) {
                                const auto& offers = mActiveMerchant->GetOffers();
                                int displayIndex = 0;
                                for (int i = 0; i < static_cast<int>(offers.size()); ++i) {
                                    const auto& offer = offers[i];
                                    if (offer.mBuyPrice == 0) continue;

                                    if (displayIndex == mTradeMenuSelection) {
                                        TradeResult result = mActiveMerchant->BuyFromMerchant(*mOverworldPlayer, offer.mItemName, 1);
                                        if (result.mStatus == TradeStatus::Success) {
                                            mPickupMessage = "Bought " + offer.mItemName + " for " + std::to_string(offer.mBuyPrice) + " gold";
                                        } else {
                                            mPickupMessage = result.mMessage;
                                        }
                                        mPickupMessageTime = SDL_GetTicks();
                                        break;
                                    }
                                    displayIndex++;
                                }
                            } else {
                                // Sell resources from world inventory to merchant for gold
                                const auto& offers = mActiveMerchant->GetOffers();
                                if (mTradeMenuSelection >= 0 && mTradeMenuSelection < static_cast<int>(offers.size())) {
                                    const auto& offer = offers[mTradeMenuSelection];

                                    // Map offer name to ItemType
                                    auto& worldInv = mOverWorld->GetInventory();
                                    ItemType type = ItemType::Wood;
                                    bool validType = true;
                                    if (offer.mItemName == "Wood") type = ItemType::Wood;
                                    else if (offer.mItemName == "Stone") type = ItemType::Stone;
                                    else if (offer.mItemName == "Metal") type = ItemType::Metal;
                                    else validType = false;

                                    if (validType && worldInv.HasEnough(type, 1)) {
                                        if (mActiveMerchant->SpendGold(offer.mSellPrice)) {
                                            worldInv.RemoveItem(type, 1);
                                            mOverworldPlayer->AddGold(offer.mSellPrice);
                                            mPickupMessage = "Sold 1 " + offer.mItemName + " for "
                                                + std::to_string(offer.mSellPrice) + " gold";
                                        } else {
                                            mPickupMessage = "Merchant can't afford that.";
                                        }
                                    } else if (!validType) {
                                        mPickupMessage = "Can't sell that item.";
                                    } else {
                                        mPickupMessage = "No " + offer.mItemName + " to sell.";
                                    }
                                    mPickupMessageTime = SDL_GetTicks();
                                }
                            }
                        }
                        if (mState == GameState::RESOURCE_MANAGEMENT && mActiveResourceManager) {
                            std::string message;
                            if (mResourceMenuTab == 0) {
                                mActiveResourceManager->UpgradeBuilding(static_cast<std::size_t>(mResourceMenuSelection),
                                                                        &message);
                            } else if (mResourceMenuTab == 1) {
                                mActiveResourceManager->HireLane(static_cast<std::size_t>(mResourceMenuSelection),
                                                                &message);
                                const WorldGrid& grid = mOverWorld->GetGrid();
                                for (size_t y = 0; y < grid.GetHeight(); ++y) {
                                    for (size_t x = 0; x < grid.GetWidth(); ++x) {
                                        WorldPosition pos(x, y);
                                        std::string cellName = grid.GetCellTypeName(grid[pos]);
                                        if (cellName == "ow_building") {
                                            cellName = "ow_grass";
                                        }
                                        mOverworldGrid->SetCell(x, y, cellName);
                                    }
                                }
                            } else {
                                const std::array<ItemType, 3> items = {ItemType::Wood, ItemType::Stone, ItemType::Metal};
                                mActiveResourceManager->SellResource(
                                        items[static_cast<std::size_t>(mResourceMenuSelection)], 1, &message);
                            }
                            mPickupMessage = message;
                            mPickupMessageTime = SDL_GetTicks();
                        }
                        break;

                case SDLK_w:
                case SDLK_s:
                case SDLK_a:
                case SDLK_d:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        if (mShowBackpack) {
                            // Navigate backpack grid
                            int rows = static_cast<int>(Inventory::BACKPACK_SIZE / Inventory::ITEMS_PER_ROW);
                            int cols = static_cast<int>(Inventory::ITEMS_PER_ROW);
                            switch (event.key.keysym.sym) {
                            case SDLK_w: mBackpackCursorRow = (mBackpackCursorRow - 1 + rows) % rows; break;
                            case SDLK_s: mBackpackCursorRow = (mBackpackCursorRow + 1) % rows; break;
                            case SDLK_a: mBackpackCursorCol = (mBackpackCursorCol - 1 + cols) % cols; break;
                            case SDLK_d: mBackpackCursorCol = (mBackpackCursorCol + 1) % cols; break;
                            default: break;
                            }
                        } else {
                            static Uint32 last_move_time = 0;
                            Uint32 now = SDL_GetTicks();
                            if (now - last_move_time >= TURN_DELAY)
                            {
                                ProcessPlayerMove(event.key.keysym.sym);
                                last_move_time = now;
                            }
                        }
                    }
                    break;

                case SDLK_TAB:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        mShowBackpack = !mShowBackpack;
                    }
                    break;

                case SDLK_x:
                    if (mState == GameState::OVERWORLD) {
                        // InteractiveWorld-only debug shortcut for upgrade testing.
                        auto& inventory = mOverWorld->GetInventory();
                        inventory.AddItem(ItemType::Wood, 1000);
                        inventory.AddItem(ItemType::Stone, 1000);
                        inventory.AddItem(ItemType::Metal, 1000);
                        mPickupMessage = "Debug: resources maxed.";
                        mPickupMessageTime = SDL_GetTicks();
                    }
                    break;

                case SDLK_e:
                    if (mState == GameState::TRADING) {
                        // Close trade menu
                        mActiveMerchant = nullptr;
                        mState = mPreviousState;
                    }
                    else if (mState == GameState::RESOURCE_MANAGEMENT) {
                        mActiveResourceManager = nullptr;
                        mState = mPreviousState;
                    }
                    else if ((mState == GameState::OVERWORLD) && !mShowBackpack)
                    {
                        // Check for adjacent merchant first
                        WorldPosition playerPos = mOverworldPlayer->GetLocation().AsWorldPosition();
                        std::array<WorldPosition, 4> adjacent = {
                            playerPos.Up(), playerPos.Down(), playerPos.Left(), playerPos.Right()
                        };

                        bool openedTrade = false;
                        bool openedResourceManager = false;
                        for (const auto& adjPos : adjacent) {
                            if (openedTrade) break;
                            for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                                AgentBase& agent = mOverWorld->GetAgentByIndex(i);
                                if (!agent.GetLocation().IsPosition()) continue;
                                if (agent.GetLocation().AsWorldPosition() != adjPos) continue;

                                if (auto* merchant = dynamic_cast<MerchantAgent*>(&agent)) {
                                    mActiveMerchant = merchant;
                                    mTradeMenuSelection = 0;
                                    mTradeBuyMode = true; // default to buy tab
                                    mPreviousState = mState;
                                    mState = GameState::TRADING;
                                    openedTrade = true;
                                    break;
                                }
                            }
                        }

                        if (!openedTrade) {
                            for (const auto& adjPos: adjacent) {
                                if (openedResourceManager)
                                    break;
                                for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                                    AgentBase& agent = mOverWorld->GetAgentByIndex(i);
                                    if (!agent.GetLocation().IsPosition())
                                        continue;
                                    if (agent.GetLocation().AsWorldPosition() != adjPos)
                                        continue;

                                    if (auto* manager = dynamic_cast<ResourceManagementAgent*>(&agent)) {
                                        mActiveResourceManager = manager;
                                        mResourceMenuSelection = 0;
                                        mResourceMenuTab = 0;
                                        mPreviousState = mState;
                                        mState = GameState::RESOURCE_MANAGEMENT;
                                        openedResourceManager = true;
                                        break;
                                    }
                                }
                            }
                        }

                        // If no merchant found, use the InteractiveWorld interaction path.
                        // This lets the GUI reach Group14 agents such as Resource Manager.
                        if (!openedTrade && !openedResourceManager) {
                            constexpr size_t interactAction = 5;
                            if (mOverWorld->DoAction(*mOverworldPlayer, interactAction)) {
                                mPickupMessage = "Interaction complete.";
                                mPickupMessageTime = SDL_GetTicks();
                                mTurnTaken = true;
                                break;
                            }

                            std::string combined;
                            for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                                AgentBase& agent = mOverWorld->GetAgentByIndex(i);
                                if (auto* spawn = dynamic_cast<ResourceSpawn*>(&agent)) {
                                    if (spawn->GetQuantity() < spawn->GetMaxCollectionQuantity() * 2) {
                                        continue;
                                    }
                                    int collected = spawn->Collect();
                                    if (collected > 0) {
                                        mOverWorld->GetInventory().AddItem(spawn->GetItemType(), collected);
                                        if (!combined.empty())
                                            combined += "  ";
                                        combined += std::to_string(collected) + " " +
                                                    std::string(ItemTypeToString(spawn->GetItemType()));
                                    }
                                }
                            }
                            if (!combined.empty()) {
                                mPickupMessage = "Collected: " + combined;
                                mPickupMessageTime = SDL_GetTicks();
                            }

                            // Try upgrade adjacent building
                            for (const auto& adjPos : adjacent) {
                                for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                                    AgentBase& agent = mOverWorld->GetAgentByIndex(i);
                                    if (!agent.GetLocation().IsPosition()) continue;
                                    if (agent.GetLocation().AsWorldPosition() != adjPos) continue;

                                    if (auto* building = dynamic_cast<Building*>(&agent)) {
                                        if (building->IsMaxLevel()) {
                                            mPickupMessage = building->GetName() + " is max level!";
                                            mPickupMessageTime = SDL_GetTicks();
                                        } else {
                                            auto upgradeInfo = building->GetNextUpgradeInfo();
                                            if (upgradeInfo) {
                                                auto& inv = mOverWorld->GetInventory();
                                                ItemType needed = upgradeInfo->item;
                                                int cost = upgradeInfo->quantity;
                                                if (inv.HasEnough(needed, cost)) {
                                                    auto result = building->Upgrade(needed, cost);
                                                    if (result) {
                                                        inv.RemoveItem(needed, cost);
                                                        mPickupMessage = building->GetName() + " upgraded to level "
                                                            + std::to_string(building->GetCurrentLevel()) + "!";
                                                    }
                                                } else {
                                                    mPickupMessage = building->GetName() + " needs "
                                                        + std::to_string(cost) + " "
                                                        + std::string(ItemTypeToString(needed))
                                                        + " (have " + std::to_string(inv.GetAmount(needed)) + ")";
                                                }
                                                mPickupMessageTime = SDL_GetTicks();
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else if (mState == GameState::DUNGEON) {
                        mDungeonWorld->DoAction(*mDungeonPlayer, DungeonActions::INTERACT);
                        mTurnTaken = true;
                    }

                    break; // end case e (finally)

                    // Just used for crown Easter Egg really, shortens the selling menu time.
                    case SDLK_r:
                        if (mState == GameState::TRADING && !mTradeBuyMode && mActiveMerchant) {
                            auto& worldInv = mOverWorld->GetInventory();
                            const auto& offers = mActiveMerchant->GetOffers();
                            size_t totalGold = 0;
                            std::string combined;

                            for (const auto& offer : offers) {
                                ItemType type = ItemType::Wood;
                                bool validType = true;
                                if (offer.mItemName == "Wood") type = ItemType::Wood;
                                else if (offer.mItemName == "Stone") type = ItemType::Stone;
                                else if (offer.mItemName == "Metal") type = ItemType::Metal;
                                else validType = false;

                                if (!validType) continue;

                                size_t have = worldInv.GetAmount(type);
                                if (have == 0) continue;

                                size_t payout = have * offer.mSellPrice;
                                if (!mActiveMerchant->SpendGold(payout)) {
                                    // Sell as much as merchant can afford
                                    have = mActiveMerchant->GetGold() / offer.mSellPrice;
                                    if (have == 0) continue;
                                    payout = have * offer.mSellPrice;
                                    mActiveMerchant->SpendGold(payout);
                                }

                                worldInv.RemoveItem(type, have);
                                mOverworldPlayer->AddGold(payout);
                                totalGold += payout;

                                if (!combined.empty()) combined += "  ";
                                combined += std::to_string(have) + " " + offer.mItemName;
                            }

                            if (totalGold > 0) {
                                mPickupMessage = "Sold " + combined + " for " + std::to_string(totalGold) + " gold";
                            } else {
                                mPickupMessage = "Nothing to sell.";
                            }
                            mPickupMessageTime = SDL_GetTicks();
                        }
                        if (mState == GameState::RESOURCE_MANAGEMENT && mActiveResourceManager && mResourceMenuTab == 2) {
                            const std::array<ItemType, 3> items = {ItemType::Wood, ItemType::Stone, ItemType::Metal};
                            ItemType item = items[static_cast<std::size_t>(mResourceMenuSelection)];
                            std::string message;
                            mActiveResourceManager->SellResource(item, mOverWorld->GetInventory().GetAmount(item),
                                                                &message);
                            mPickupMessage = message;
                            mPickupMessageTime = SDL_GetTicks();
                        }
                    break;

                // Number keys 0-9: move backpack item to hotbar slot
                case SDLK_0:
                case SDLK_1:
                case SDLK_2:
                case SDLK_3:
                case SDLK_4:
                case SDLK_5:
                case SDLK_6:
                case SDLK_7:
                case SDLK_8:
                case SDLK_9:
                    if (mShowBackpack && (mState == GameState::OVERWORLD || mState == GameState::DUNGEON))
                    {
                        // SDLK_1 = slot 0, SDLK_2 = slot 1, etc.
                        size_t hotbar_slot = (event.key.keysym.sym == SDLK_0) ? 9 : static_cast<size_t>(event.key.keysym.sym - SDLK_1);
                        if (hotbar_slot < Inventory::HOTBAR_SIZE) {
                            size_t backpack_index = Inventory::HOTBAR_SIZE
                                + mBackpackCursorRow * static_cast<int>(Inventory::ITEMS_PER_ROW)
                                + mBackpackCursorCol;

                            Inventory& inv = (mState == GameState::OVERWORLD)
                                ? mOverworldPlayer->GetInventory()
                                : mDungeonPlayer->GetInventory();

                            inv.SwapSlots(backpack_index, hotbar_slot);
                        }
                    }
                    break;

                // Pause / resume
                case SDLK_ESCAPE:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        Pause();
                    }
                    else if (mState == GameState::PAUSED)
                    {
                        Resume();
                    }
                    else if (mState == GameState::CONTROLS || mState == GameState::STATS)
                    {
                        Resume();
                    }
                    else if (mState == GameState::TRADING)
                    {
                        mActiveMerchant = nullptr;
                        mState = mPreviousState;
                    }
                    else if (mState == GameState::RESOURCE_MANAGEMENT) {
                        mActiveResourceManager = nullptr;
                        mState = mPreviousState;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    //  State Transitions
    // -----------------------------------------------------------------------

    void Game::TransitionTo(GameState new_state)
    {
        if ((mPreviousState == GameState::DUNGEON || mPreviousState == GameState::OVERWORLD) &&
            new_state == GameState::MAIN_MENU) {
            mAnalyticsManager->LogDamageDealt(mAnalyticsManager->GetCurrentRunStats().damageDealt);
            mAnalyticsManager->LogEnemiesKilled(mAnalyticsManager->GetCurrentRunStats().enemiesKilled);
            mAnalyticsManager->ResetCurrentRunStats();
            }

        // Don't overwrite mPreviousState when leaving PAUSED for a sub-screen
        if (mState != GameState::PAUSED && mState != GameState::STATS && mState != GameState::CONTROLS) {
            mPreviousState = mState;
        }
        mState = new_state;
    }

    void Game::Pause()
    {
        mPreviousState = mState;
        mState = GameState::PAUSED;
        mPauseMenu.SelectOption(0); // Always start pause menu on "Resume"
    }

    void Game::Resume() { mState = mPreviousState; }

    // -----------------------------------------------------------------------
    //  Update
    // -----------------------------------------------------------------------

    void Game::UpdateMainMenu() {}


    void Game::UpdateOverworld()
    {
        // Producers use their own burst intervals, so ticking every frame does
        // not create resources until an interval has elapsed.
        for (const auto& producer: mOverWorld->GetProducers()) {
            producer->Update();
        }

        const Uint32 now = SDL_GetTicks();
        if (now - mLastOverworldAgentTick >= OVERWORLD_AGENT_STEP_DELAY) {
            mLastOverworldAgentTick = now;
            for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                AgentBase& agent = mOverWorld->GetAgentByIndex(i);
                if (&agent == mOverworldPlayer)
                    continue;

                size_t action = agent.SelectAction(mOverWorld->GetGrid());
                mOverWorld->DoAction(agent, action);
            }
        }

        mTurnTaken = false;
    }


    void Game::UpdateDungeon()
    {
        // skip the player in the world agent list, they should choose their own move when needed to.
        if (mTurnTaken) {
            for (size_t i = 0; i < mDungeonWorld->GetNumAgents(); ++i) {
                // KAREN: I believe this should be GetAgentByIndex
                // AgentBase& agent = mDungeonWorld->GetAgent(i);
                AgentBase& agent = mDungeonWorld->GetAgentByIndex(i);

                if (&agent == mDungeonPlayer) continue;
                // KAREN: I added the following:
                if (!agent.IsAlive()) continue;
                if (agent.IsPlayerAgent()) continue;

                size_t action = agent.SelectAction(mDungeonWorld->GetGrid());
                mDungeonWorld->DoAction(agent, action);
            }

            mDungeonWorld->RemoveDeadAgents(); // KAREN: just in case...
            mDungeonWorld->CleanupSpawnedEnemyIds(); // mark, moved out of inner in dungeon world to here
            mTurnTaken = false;
        }

        // Transfer dungeon gold earnings to Interactive World Inventory
        size_t gold = mDungeonWorld->DrainAccumulatedGold();
        if (gold > 0) {
            mOverworldPlayer->AddGold(gold);
            mPickupMessage = "Gained " + std::to_string(gold) + " gold from defeated enemy!";
            mPickupMessageTime = SDL_GetTicks();
        }
    }

    void Game::UpdatePaused() {}
    void Game::UpdateControls() {}

    void Game::UpdateStats() {
        // do nothing, should only sync on entry
    }

    // -----------------------------------------------------------------------
    //  Render
    // -----------------------------------------------------------------------

    void Game::RenderMainMenu()
    {
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        int menu_w = w / 4;
        int menu_h = static_cast<int>(mMainMenu.GetOptionCount()) * 50;
        int menu_x = (w - menu_w) / 2;
        int menu_y = (h - menu_h) / 2;

        int title_x = (w - mTitleText.GetWidth()) / 2;
        mTitleText.Draw(title_x, menu_y - 80);

        SDL_Renderer *renderer = mGameView->GetRenderer();
        mMainMenu.DrawMenu(renderer, menu_x, menu_y, menu_w, menu_h);
    }

    // Draw base terrain first, then InteractiveWorld agents/objects. Locked
    // lane objects retain world positions but are skipped until purchased.
    void Game::RenderOverworld() {
        RenderWorld(*mOverworldGrid, mCamX, mCamY);

        int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
        int th = static_cast<int>(mOverworldGrid->GetTileHeight());

        bool quarryLaneUnlocked = false;
        bool mineLaneUnlocked = false;
        for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
            if (auto* manager = dynamic_cast<ResourceManagementAgent*>(&mOverWorld->GetAgentByIndex(i))) {
                for (std::size_t laneIndex = 0; laneIndex < manager->GetHireableLaneCount(); ++laneIndex) {
                    if (manager->GetHireableLaneLabel(laneIndex) == "Quarry Lane") {
                        quarryLaneUnlocked = manager->IsLaneUnlocked(laneIndex);
                    } else if (manager->GetHireableLaneLabel(laneIndex) == "Mine Lane") {
                        mineLaneUnlocked = manager->IsLaneUnlocked(laneIndex);
                    }
                }
            }
        }

        for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
            AgentBase& agent = mOverWorld->GetAgentByIndex(i);
            if (!agent.GetLocation().IsPosition())
                continue;
            if (const auto* fetcher = dynamic_cast<FetchAgent*>(&agent); fetcher != nullptr && !fetcher->IsActive())
                continue;
            if ((agent.GetName() == "Quarry" || agent.GetName() == "Stone Spawn") && !quarryLaneUnlocked)
                continue;
            if ((agent.GetName() == "Mine" || agent.GetName() == "Ore Mine" || agent.GetName() == "Metal Spawn") &&
                !mineLaneUnlocked)
                continue;

            const WorldPosition& pos = agent.GetLocation().AsWorldPosition();

            int screen_x = (static_cast<int>(pos.CellX()) - mCamX) * tw;
            int screen_y = (static_cast<int>(pos.CellY()) - mCamY) * th;

            std::string sprite = "ow_grass";
            if (&agent == mOverworldPlayer) {
                sprite = "interactive_player";
            } else if (agent.GetName() == "Explorer") {
                sprite = "goblin";
            } else if (agent.GetName() == "Town Hall") {
                sprite = "ow_town_hall";
            } else if (auto* building = dynamic_cast<Building*>(&agent)) {
                const std::string& name = agent.GetName();
                if (name == "Lumber Yard") {
                    sprite = building->GetCurrentLevel() > 0 ? "ow_building_lumberyard_upgraded" : "ow_building_lumberyard";
                } else if (name == "Quarry") {
                    sprite = building->GetCurrentLevel() > 0 ? "ow_building_quarry_upgraded" : "ow_building_quarry";
                } else if (name == "Mine" || name == "Ore Mine") {
                    sprite = building->GetCurrentLevel() > 0 ? "ow_building_mine_upgraded" : "ow_building_mine";
                } else {
                    sprite = "ow_building_lumberyard";
                }
            } else if (auto* spawn = dynamic_cast<ResourceSpawn*>(&agent)) {
                // Resource asset states are generated as empty, partial, full.
                // The middle state uses the base resource_*_spawn sprite.
                const int fullThreshold = spawn->GetMaxCollectionQuantity() * 2;
                const std::string stateSuffix = spawn->GetQuantity() == 0               ? "_empty"
                                                : spawn->GetQuantity() >= fullThreshold ? "_full"
                                                                                        : "";
                switch (spawn->GetItemType()) {
                    case ItemType::Wood:
                        sprite = "resource_wood_spawn" + stateSuffix;
                        break;
                    case ItemType::Stone:
                        sprite = "resource_stone_spawn" + stateSuffix;
                        break;
                    case ItemType::Metal:
                        sprite = "resource_metal_spawn" + stateSuffix;
                        break;
                }
            } else if (dynamic_cast<ResourceManagementAgent*>(&agent)) {
                sprite = "resource_manager";
            } else if (dynamic_cast<FetchAgent*>(&agent)) {
                sprite = "fetch_agent";
            } else if (agent.GetName() == "Trader" || agent.GetName() == "Farmer") {
                sprite = "player"; // use player sprite as placeholder for trader and farmer
            } else {
                sprite = "skeleton";
            }

            mImageManager->DrawImage(sprite, screen_x, screen_y, tw, th);
        }

        // World resource inventory at top
        RenderWorldInventory();
        RenderPickupMessage();
    }

    void Game::RenderDungeon()
    {
        RenderWorld(*mDungeonGrid, mDungeonCamX, mDungeonCamY);

        // Draw floor underneath loot chest tiles so they don't have a black background
        {
            int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
            int th = static_cast<int>(mDungeonGrid->GetTileHeight());
            const WorldGrid& grid = mDungeonWorld->GetGrid();
            std::string floorName;
            switch (mDungeonWorld->GetLevel()) {
                case 1:  floorName = "floor_l1v1"; break;
                case 2:  floorName = "floor_l2v1"; break;
                case 3:  floorName = "floor_l3v1"; break;
                default: floorName = "floor_l1v1"; break;
            }

            for (size_t y = 0; y < grid.GetHeight(); ++y) {
                for (size_t x = 0; x < grid.GetWidth(); ++x) {
                    WorldPosition pos(x, y);
                    if (grid.GetCellTypeName(grid[pos]) == "wall_loot") {
                        int screen_x = (static_cast<int>(x) - mDungeonCamX) * tw;
                        int screen_y = (static_cast<int>(y) - mDungeonCamY) * th;
                        mImageManager->DrawImage(floorName, screen_x, screen_y, tw, th);
                        mImageManager->DrawImage("wall_loot", screen_x, screen_y, tw, th);
                    }
                }
            }
        }

        int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
        int th = static_cast<int>(mDungeonGrid->GetTileHeight());

        for (size_t i = 0; i < mDungeonWorld->GetNumAgents(); ++i)
        {
            const AgentBase &agent = mDungeonWorld->GetAgentByIndex(i);
            const WorldPosition &pos = agent.GetLocation().AsWorldPosition();

            int screen_x = (static_cast<int>(pos.CellX()) - mDungeonCamX) * tw;
            int screen_y = (static_cast<int>(pos.CellY()) - mDungeonCamY) * th;

            // const std::string &sprite = (&agent == mDungeonPlayer) ? "player" : "dun_monster";
            std::string sprite;
            if (&agent == mDungeonPlayer) {
                sprite = "player";
            } else if (agent.GetName().rfind("Goblin", 0) == 0) {
                sprite = "goblin";
            } else {
                sprite = "dun_monster";
            }
            mImageManager->DrawImage(sprite, screen_x, screen_y, tw, th);
        }

        // Player health display
        {
            double currentHP = mDungeonPlayer->GetCurrentHealth();
            double maxHP = mDungeonPlayer->GetMaxHealth();

            std::string healthText = "HP: " + std::to_string(static_cast<int>(currentHP))
                                   + " / " + std::to_string(static_cast<int>(maxHP));

            mPickupText.SetSize(18);
            mPickupText.SetBold(true);
            mPickupText.SetContent(healthText);
            mPickupText.Draw(80, mGameView->GetHeight() - 100);
        }

        RenderHotbar(mDungeonPlayer->GetInventory());
        RenderPickupMessage();
        if (mShowBackpack) RenderBackpack(mDungeonPlayer->GetInventory());
    }

    void Game::RenderWorld(const ImageGrid &grid, int camX, int camY)
    {
        grid.DrawViewport(*mImageManager, camX, camY, mGameView->GetWidth(), mGameView->GetHeight());
    }

    void Game::RenderPaused()
    {
        SDL_Renderer *renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Semi-transparent dark overlay over whatever world was active
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
        SDL_Rect overlay = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &overlay);

        // "Paused" title
        int pause_x = (w - mPauseText.GetWidth()) / 2;
        int pause_y = h / 4;
        mPauseText.Draw(pause_x, pause_y);

        // Pause menu centered
        int menu_w = w / 4;
        int menu_h = static_cast<int>(mPauseMenu.GetOptionCount()) * 50;
        int menu_x = (w - menu_w) / 2;
        int menu_y = pause_y + mPauseText.GetHeight() + (h / 30);
        mPauseMenu.DrawMenu(renderer, menu_x, menu_y, menu_w, menu_h);
    }

    void Game::RenderControls()
    {
        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_Rect bg = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &bg);

        int y = 30;
        const int LINE_H = 24;

        mStatsText.SetContent("Controls");
        mStatsText.SetSize(28);
        mStatsText.SetBold(true);
        mStatsText.Draw((w - mStatsText.GetWidth()) / 2, y);
        y += LINE_H * 2;

        mStatsText.SetSize(14);
        mStatsText.SetBold(false);

        auto drawLine = [&](const std::string& text) {
            mStatsText.SetContent(text);
            mStatsText.Draw(60, y);
            y += LINE_H;
        };

        auto drawHeader = [&](const std::string& text) {
            y += LINE_H / 2;
            mStatsText.SetBold(true);
            mStatsText.SetContent(text);
            mStatsText.Draw(60, y);
            y += LINE_H;
            mStatsText.SetBold(false);
        };

        drawHeader("Movement & General");
        drawLine("W/A/S/D - Move    E - Interact/Attack    TAB - Backpack    1-0 - Equip item");
        drawLine("ESC - Pause    Arrows - Navigate menus    Enter - Confirm");

        drawHeader("Overworld");
        drawLine("E - Collect resources / Upgrade buildings / Open trade menu");
        drawLine("Left/Right - Buy/Sell tabs    Up/Down - Browse    Enter - Confirm    R - Sell all");

        drawHeader("Dungeon");
        drawLine("E - Attack adjacent enemy / Interact with exit door");
        drawLine("Walk into exit door to advance to next level");

        y = h - 40;
        mStatsText.SetSize(12);
        mStatsText.SetContent("Press ESC to return");
        mStatsText.Draw((w - mStatsText.GetWidth()) / 2, y);
    }

    void Game::RenderStats() {
        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Dark background
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_Rect bg = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &bg);

        // Vals for spacing
        int y = 40;
        const int LINE_H = 36;

        // Title
        mStatsText.SetContent("Stats");
        mStatsText.SetSize(36);
        mStatsText.SetBold(true);
        mStatsText.Draw((w - mStatsText.GetWidth()) / 2, y);
        y += LINE_H * 2;

        // Numeric stats
        for (const StatSummary& stat : mDashboardSnapshot.numericStats) {
            mStatsText.SetContent(stat.label + ":");
            mStatsText.SetSize(22);
            mStatsText.SetBold(true);
            mStatsText.Draw(60, y);
            y += LINE_H;

            mStatsText.SetContent("  Current : " + std::to_string(static_cast<int>(stat.currentValue)));
            mStatsText.SetBold(false);
            mStatsText.Draw(60, y);
            y += LINE_H;

            std::string detail;
            if (stat.minValue)  detail += "Min: "  + std::to_string(static_cast<int>(*stat.minValue))  + "  ";
            if (stat.maxValue)  detail += "Max: "  + std::to_string(static_cast<int>(*stat.maxValue))  + "  ";
            if (stat.meanValue) detail += "Mean: " + std::to_string(static_cast<int>(*stat.meanValue));
            if (!detail.empty()) {
                mStatsText.SetContent("  " + detail);
                mStatsText.Draw(60, y);
                y += LINE_H;
            }

            mStatsText.SetContent("  Runs logged: " + std::to_string(stat.sampleCount));
            mStatsText.Draw(60, y);
            y += LINE_H + 8;
        }

        // Action stats
        for (const ActionSummary& action : mDashboardSnapshot.actionStats) {
            mStatsText.SetContent(action.label + ":");
            mStatsText.SetSize(22);
            mStatsText.SetBold(true);
            mStatsText.Draw(60, y);
            y += LINE_H;

            mStatsText.SetBold(false);
            mStatsText.SetContent("  Total actions: " + std::to_string(action.actionCount));
            mStatsText.Draw(60, y);
            y += LINE_H;

            if (action.mostActiveEntity) {
                mStatsText.SetContent("  Most active entity ID: " + std::to_string(*action.mostActiveEntity));
                mStatsText.Draw(60, y);
                y += LINE_H;
            }
            y += 8;
        }

        // Empty state
        if (mDashboardSnapshot.numericStats.empty() && mDashboardSnapshot.actionStats.empty()) {
            mStatsText.SetContent("No stats recorded yet.");
            mStatsText.SetSize(22);
            mStatsText.SetBold(false);
            mStatsText.Draw((w - mStatsText.GetWidth()) / 2, y);
        }
    }

    void Game::RenderHotbar(const Inventory &inventory) {
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Draw the inventory bar image centered at the bottom
        int bar_w = 640;
        int bar_h = 64;
        int bar_x = (w - bar_w) / 2;
        int bar_y = h - bar_h - 10; // 10px padding from bottom

        mImageManager->DrawImage("inventory_bar", bar_x, bar_y, bar_w, bar_h);

        // Draw items in each hotbar slot
        const auto& slots = inventory.GetInventoryArray();
        int slot_size = bar_w / static_cast<int>(Inventory::HOTBAR_SIZE);

        for (size_t i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
            const auto& slot = slots[i];
            if (!slot.IsEmpty()) {
                const Item* item = slot.GetItem();
                int item_x = bar_x + static_cast<int>(i) * slot_size + (slot_size - ICON_SIZE) / 2;
                int item_y = bar_y + (bar_h - ICON_SIZE) / 2;

                // Draw item icon if loaded — uses the item's image path as key
                mImageManager->DrawImage(item->GetName(), item_x, item_y, ICON_SIZE, ICON_SIZE);
            }
        }

        // Highlight the selected hotbar slot
        SDL_Renderer* renderer = mGameView->GetRenderer();
        size_t selected =  inventory.GetHandSlotIndex();
        int sel_x = bar_x + static_cast<int>(selected) * slot_size;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect highlight = {sel_x, bar_y, slot_size, bar_h};
        SDL_RenderDrawRect(renderer, &highlight);
    }

    void Game::RenderBackpack(const Inventory& inventory) {
        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Semi-transparent dark overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &overlay);

        const auto& slots = inventory.GetInventoryArray();

        int slot_size = 64;
        int padding = 4;
        int cols = static_cast<int>(Inventory::ITEMS_PER_ROW);
        int rows = static_cast<int>(Inventory::BACKPACK_SIZE) / cols;

        int grid_w = cols * (slot_size + padding) - padding;
        int grid_h = rows * (slot_size + padding) - padding;
        int grid_x = (w - grid_w) / 2;
        int grid_y = (h - grid_h) / 2;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                size_t index = Inventory::HOTBAR_SIZE + row * cols + col;

                int x = grid_x + col * (slot_size + padding);
                int y = grid_y + row * (slot_size + padding);

                // Slot background
                SDL_SetRenderDrawColor(renderer, 40, 40, 50, 220);
                SDL_Rect slot_bg = {x, y, slot_size, slot_size};
                SDL_RenderFillRect(renderer, &slot_bg);

                // Slot border
                SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
                SDL_RenderDrawRect(renderer, &slot_bg);

                // Draw item if present
                if (!slots[index].IsEmpty()) {
                    const Item* item = slots[index].GetItem();
                    int item_x = x + (slot_size - ICON_SIZE) / 2;
                    int item_y = y + (slot_size - ICON_SIZE) / 2;
                    mImageManager->DrawImage(item->GetName(), item_x, item_y, ICON_SIZE, ICON_SIZE);
                }

                // Cursor highlight
                if (row == mBackpackCursorRow && col == mBackpackCursorCol) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 220);
                    SDL_Rect cursor = {x - 2, y - 2, slot_size + 4, slot_size + 4};
                    SDL_RenderDrawRect(renderer, &cursor);
                    SDL_Rect cursor_inner = {x - 1, y - 1, slot_size + 2, slot_size + 2};
                    SDL_RenderDrawRect(renderer, &cursor_inner);
                }
            }
        }
    }

    void Game::RenderWorldInventory() {
            SDL_Renderer* renderer = mGameView->GetRenderer();
            int w = mGameView->GetWidth();

            // Background bar at top
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
            SDL_Rect bg = {0, 0, w, 40};
            SDL_RenderFillRect(renderer, &bg);

            const auto& inv = mOverWorld->GetInventory();

            std::string text = "Gold: " + std::to_string(mOverworldPlayer->GetGold())
                 + "   Wood: " + std::to_string(inv.GetAmount(ItemType::Wood))
                 + "   Stone: " + std::to_string(inv.GetAmount(ItemType::Stone))
                 + "   Metal: " + std::to_string(inv.GetAmount(ItemType::Metal));

            mPickupText.SetContent(text);
            mPickupText.SetSize(18);
            mPickupText.SetBold(false);
            int text_x = (w - mPickupText.GetWidth()) / 2;
            mPickupText.Draw(text_x, 10);
    }

    void Game::RenderPickupMessage() {
        if (mPickupMessage.empty()) return;

        Uint32 elapsed = SDL_GetTicks() - mPickupMessageTime;
        if (elapsed > PICKUP_MESSAGE_DURATION_MS) {
            mPickupMessage.clear();
            return;
        }

        int w = mGameView->GetWidth();

        mPickupText.SetContent(mPickupMessage);
        int text_x = (w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, 40);
    }


    void Game::UpdateTrading() {
        // Nothing to update — trade menu is event-driven
        // Put here for consistency or possible stat logging.
    }

    void Game::UpdateResourceManagement() {
        // Resource management is event-driven.
    }

    void Game::RenderTrading() {
        RenderOverworld();

        if (!mActiveMerchant) return;

        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Dark overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &overlay);

        int panel_w = 420;
        int panel_h = 380;
        int panel_x = (w - panel_w) / 2;
        int panel_y = (h - panel_h) / 2;

        // Panel background
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 240);
        SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
        SDL_RenderFillRect(renderer, &panel);
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_RenderDrawRect(renderer, &panel);

        int y = panel_y + 15;

        // Merchant greeting
        mPickupText.SetSize(20);
        mPickupText.SetBold(true);
        mPickupText.SetContent(mActiveMerchant->GetTradeGreeting());
        int text_x = panel_x + (panel_w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, y);
        y += 30;

        // Buy/Sell tabs
        mPickupText.SetSize(18);
        std::string buyLabel = mTradeBuyMode ? "[ BUY ]" : "  BUY  ";
        std::string sellLabel = !mTradeBuyMode ? "[ SELL ]" : "  SELL  ";
        mPickupText.SetBold(mTradeBuyMode);
        mPickupText.SetContent(buyLabel);
        mPickupText.Draw(panel_x + panel_w / 4 - mPickupText.GetWidth() / 2, y);
        mPickupText.SetBold(!mTradeBuyMode);
        mPickupText.SetContent(sellLabel);
        mPickupText.Draw(panel_x + 3 * panel_w / 4 - mPickupText.GetWidth() / 2, y);
        y += 30;

        // Player gold
        mPickupText.SetSize(16);
        mPickupText.SetBold(false);
        mPickupText.SetContent("Gold: " + std::to_string(mOverworldPlayer->GetGold()));
        mPickupText.Draw(panel_x + 20, y);
        y += 25;

        if (mTradeBuyMode) {
            const auto& offers = mActiveMerchant->GetOffers();
            int displayIndex = 0;
            for (int i = 0; i < static_cast<int>(offers.size()); ++i) {
                const auto& offer = offers[i];
                if (offer.mBuyPrice == 0) continue; // not for sale

                std::string line = offer.mItemName + "  -  " + std::to_string(offer.mBuyPrice) + " gold";
                if (offer.mStockMode == TradeStockMode::Limited) {
                    line += "  [" + std::to_string(offer.mStock) + " left]";
                }

                mPickupText.SetSize(16);
                mPickupText.SetBold(displayIndex == mTradeMenuSelection);
                mPickupText.SetContent(line);
                mPickupText.Draw(panel_x + 30, y);

                if (displayIndex == mTradeMenuSelection) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
                    SDL_Rect sel = {panel_x + 10, y - 2, panel_w - 20, 22};
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_RenderDrawRect(renderer, &sel);
                }
                y += 25;
                displayIndex++;
            }

            if (displayIndex == 0) {
                mPickupText.SetSize(16);
                mPickupText.SetBold(false);
                mPickupText.SetContent("No items for sale.");
                mPickupText.Draw(panel_x + 30, y);
            }
        } else {
            // Show world resources available to sell
            const auto& worldInv = mOverWorld->GetInventory();
            const auto& offers = mActiveMerchant->GetOffers();

            for (int i = 0; i < static_cast<int>(offers.size()); ++i) {
                const auto& offer = offers[i];

                // Get quantity from world inventory
                size_t have = 0;
                if (offer.mItemName == "Wood") have = worldInv.GetAmount(ItemType::Wood);
                else if (offer.mItemName == "Stone") have = worldInv.GetAmount(ItemType::Stone);
                else if (offer.mItemName == "Metal") have = worldInv.GetAmount(ItemType::Metal);

                std::string line = offer.mItemName + "  -  " + std::to_string(offer.mSellPrice)
                    + " gold  (have " + std::to_string(have) + ")";

                mPickupText.SetSize(16);
                mPickupText.SetBold(i == mTradeMenuSelection);
                mPickupText.SetContent(line);
                mPickupText.Draw(panel_x + 30, y);

                if (i == mTradeMenuSelection) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
                    SDL_Rect sel = {panel_x + 10, y - 2, panel_w - 20, 22};
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_RenderDrawRect(renderer, &sel);
                }
                y += 25;
            }
        }

        // Instructions
        y = panel_y + panel_h - 30;
        mPickupText.SetSize(12);
        mPickupText.SetBold(false);
        mPickupText.SetContent("LEFT/RIGHT:  buy/sell  UP/DOWN: browse  ENTER: confirm  R: sell all  E: close");
        text_x = panel_x + (panel_w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, y);
    }
    
    void Game::RenderResourceManagement() {
        RenderOverworld();

        if (!mActiveResourceManager)
            return;

        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &overlay);

        int panel_w = 520;
        int panel_h = 410;
        int panel_x = (w - panel_w) / 2;
        int panel_y = (h - panel_h) / 2;

        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 240);
        SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
        SDL_RenderFillRect(renderer, &panel);
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_RenderDrawRect(renderer, &panel);

        int y = panel_y + 15;

        mPickupText.SetSize(20);
        mPickupText.SetBold(true);
        mPickupText.SetContent("Resource Management");
        int text_x = panel_x + (panel_w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, y);
        y += 30;

        const std::array<std::string, 3> tabs = {"UPGRADE", "LANES", "SELL"};
        mPickupText.SetSize(18);
        for (int i = 0; i < 3; ++i) {
            mPickupText.SetBold(mResourceMenuTab == i);
            mPickupText.SetContent(mResourceMenuTab == i ? "[ " + tabs[i] + " ]" : "  " + tabs[i] + "  ");
            mPickupText.Draw(panel_x + (i + 1) * panel_w / 4 - mPickupText.GetWidth() / 2, y);
        }
        y += 34;

        const auto& inv = mOverWorld->GetInventory();
        mPickupText.SetSize(15);
        mPickupText.SetBold(false);
        mPickupText.SetContent("Stored: Wood " + std::to_string(inv.GetAmount(ItemType::Wood)) + "   Stone " +
                            std::to_string(inv.GetAmount(ItemType::Stone)) + "   Metal " +
                            std::to_string(inv.GetAmount(ItemType::Metal)) + "   Gold " +
                            std::to_string(mActiveResourceManager->GetGold()));
        mPickupText.Draw(panel_x + 20, y);
        y += 28;

        if (mResourceMenuTab == 0) {
            auto buildings = mOverWorld->GetBuildings();
            for (int i = 0; i < static_cast<int>(buildings.size()); ++i) {
                Building* building = buildings[static_cast<std::size_t>(i)];
                std::string line = building->GetName() + "  level " + std::to_string(building->GetCurrentLevel()) + "/" +
                                std::to_string(building->GetMaxLevel());

                if (!mActiveResourceManager->IsManagedBuildingUnlocked(static_cast<std::size_t>(i))) {
                    line += "  locked";
                } else if (auto upgrade = building->GetNextUpgradeInfo()) {
                    line += "  next: " + std::to_string(upgrade->quantity) + " " +
                            std::string(ItemTypeToString(upgrade->item));
                } else {
                    line += "  max level";
                }

                mPickupText.SetSize(16);
                mPickupText.SetBold(i == mResourceMenuSelection);
                mPickupText.SetContent(line);
                mPickupText.Draw(panel_x + 30, y);

                if (i == mResourceMenuSelection) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
                    SDL_Rect sel = {panel_x + 10, y - 2, panel_w - 20, 22};
                    SDL_RenderDrawRect(renderer, &sel);
                }
                y += 27;
            }
        } else if (mResourceMenuTab == 1) {
            for (int i = 0; i < static_cast<int>(mActiveResourceManager->GetHireableLaneCount()); ++i) {
                const bool unlocked = mActiveResourceManager->IsLaneUnlocked(static_cast<std::size_t>(i));
                std::string line =
                        mActiveResourceManager->GetHireableLaneLabel(static_cast<std::size_t>(i)) + "  cost " +
                        std::to_string(mActiveResourceManager->GetHireableLaneCost(static_cast<std::size_t>(i))) +
                        " gold  " + (unlocked ? "active" : "locked");

                mPickupText.SetSize(16);
                mPickupText.SetBold(i == mResourceMenuSelection);
                mPickupText.SetContent(line);
                mPickupText.Draw(panel_x + 30, y);

                if (i == mResourceMenuSelection) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
                    SDL_Rect sel = {panel_x + 10, y - 2, panel_w - 20, 22};
                    SDL_RenderDrawRect(renderer, &sel);
                }
                y += 27;
            }
        } else {
            const std::array<ItemType, 3> items = {ItemType::Wood, ItemType::Stone, ItemType::Metal};
            for (int i = 0; i < 3; ++i) {
                ItemType item = items[static_cast<std::size_t>(i)];
                std::string line = std::string(ItemTypeToString(item)) + "  " +
                                std::to_string(mActiveResourceManager->GetSellPrice(item)) + " gold each  (have " +
                                std::to_string(inv.GetAmount(item)) + ")";

                mPickupText.SetSize(16);
                mPickupText.SetBold(i == mResourceMenuSelection);
                mPickupText.SetContent(line);
                mPickupText.Draw(panel_x + 30, y);

                if (i == mResourceMenuSelection) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 180);
                    SDL_Rect sel = {panel_x + 10, y - 2, panel_w - 20, 22};
                    SDL_RenderDrawRect(renderer, &sel);
                }
                y += 27;
            }
        }

        y = panel_y + panel_h - 30;
        mPickupText.SetSize(12);
        mPickupText.SetBold(false);
        mPickupText.SetContent("LEFT/RIGHT: tabs  UP/DOWN: browse  ENTER: confirm  R: sell all selected  E: close");
        text_x = panel_x + (panel_w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, y);
    }

    size_t Game::KeyToAction(SDL_Keycode key) const{
        switch (key) {
        case SDLK_w: return 1; // MOVE_UP
        case SDLK_s: return 2; // MOVE_DOWN
        case SDLK_a: return 3; // MOVE_LEFT
        case SDLK_d: return 4; // MOVE_RIGHT
        default:     return 0; // REMAIN_STILL
        }
    }

    void Game::ProcessPlayerMove(SDL_Keycode key) {
        size_t action = KeyToAction(key);
        if (action == 0) return;

        if (mState == GameState::OVERWORLD) {
            mOverWorld->DoAction(*mOverworldPlayer, action);

            WorldPosition pos = mOverworldPlayer->GetLocation().AsWorldPosition();
            mPlayerX = static_cast<int>(pos.CellX());
            mPlayerY = static_cast<int>(pos.CellY());

            int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
            int th = static_cast<int>(mOverworldGrid->GetTileHeight());
            int tiles_x = mGameView->GetWidth() / tw;
            int tiles_y = mGameView->GetHeight() / th;
            int max_cam_x = std::max(0, static_cast<int>(mOverworldGrid->GetWidth()) - tiles_x);
            int max_cam_y = std::max(0, static_cast<int>(mOverworldGrid->GetHeight()) - tiles_y);

            mCamX = std::clamp(mPlayerX - tiles_x / 2, 0, max_cam_x);
            mCamY = std::clamp(mPlayerY - tiles_y / 2, 0, max_cam_y);
        }

        else if (mState == GameState::DUNGEON) {
            // Snapshot inventory count before move
            size_t items_before = 0;
            const auto& slots = mDungeonPlayer->GetInventory().GetInventoryArray();
            for (const auto& slot : slots) {
                if (!slot.IsEmpty()) items_before += slot.GetQuantity();
            }

            WorldPosition pos_before = mDungeonPlayer->GetLocation().AsWorldPosition();

            mDungeonWorld->DoAction(*mDungeonPlayer, action);

            WorldPosition pos = mDungeonPlayer->GetLocation().AsWorldPosition();
            mDungeonPlayerX = static_cast<int>(pos.CellX());
            mDungeonPlayerY = static_cast<int>(pos.CellY());

            // Detect level change — player was moved to (1,1) and grid was regenerated
            if (pos.CellX() == DUNGEON_SPAWN_X && pos.CellY() == DUNGEON_SPAWN_Y && pos_before.CellX()
            != DUNGEON_SPAWN_X && pos_before.CellY() != DUNGEON_SPAWN_Y) {
                RebuildDungeonGrid();
                mPickupMessage = "Entering next level...";
                mPickupMessageTime = SDL_GetTicks();
            }

            // Check if inventory changed
            size_t items_after = 0;
            for (const auto& slot : slots) {
                if (!slot.IsEmpty()) items_after += slot.GetQuantity();
            }
            if (items_after > items_before) {
                for (const auto& slot : slots) {
                    if (!slot.IsEmpty()) {
                        mPickupMessage = "Picked up: " + slot.GetItem()->GetName();
                    }
                }
                mPickupMessageTime = SDL_GetTicks();
            }

            // Update camera
            int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
            int th = static_cast<int>(mDungeonGrid->GetTileHeight());
            int tiles_x = mGameView->GetWidth() / tw;
            int tiles_y = mGameView->GetHeight() / th;
            int max_cam_x = std::max(0, static_cast<int>(mDungeonGrid->GetWidth()) - tiles_x);
            int max_cam_y = std::max(0, static_cast<int>(mDungeonGrid->GetHeight()) - tiles_y);

            mDungeonCamX = std::clamp(mDungeonPlayerX - tiles_x / 2, 0, max_cam_x);
            mDungeonCamY = std::clamp(mDungeonPlayerY - tiles_y / 2, 0, max_cam_y);
        }

        mTurnTaken = true;
    }

} // namespace cse498