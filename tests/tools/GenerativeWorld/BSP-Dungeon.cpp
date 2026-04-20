
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/Worlds/Dungeon/BSP-Dungeon.hpp"

#include "../../../source/Worlds/Dungeon/ForestLevel.hpp"

/// @brief Brought over from DungeonOne for test case purposes
/// @return vector of 
// inline static constexpr std::array<std::pair<std::size_t, double>, 6> ROOM_DATA{{
// 		{1, 10},
// 		{2, 10},
// 		{3, 10},
// 		{4, 10},
// 		{5, 10},
// 		{6, 1},
// 	}};

// inline static const std::string PREFIX = "one_pool/room_";

// static cse498::WeightedSet<std::string> MakeRoomPool() {
// 	cse498::WeightedSet<std::string> rooms;
	
// 	for (const auto& [num, weight] : ROOM_DATA) {
// 		auto result = rooms.Insert(PREFIX + std::to_string(num) + ".txt", weight);
// 		assert(result.has_value());
// 	}

// 	return rooms;
// }



/// @brief Setting the BSP Tree to its default state based off the values within the class itself 
/// @param BSP 
void SetDefaultSetting(cse498::BSP& BSP) {
    BSP.SetWidth(100);
    BSP.SetHeight(100);
    BSP.SetIterations(20);
}

void BSPSettingOne(cse498::BSP& BSP) {
    BSP.SetWidth(150);
    BSP.SetHeight(150);
    BSP.SetIterations(8);
}

void BSPSettingTwo(cse498::BSP& BSP) {
    BSP.SetWidth(50);
    BSP.SetHeight(50);
    BSP.SetIterations(2);
}



const uint64_t SEED_VALUE_ONE = 12345;
const uint64_t SEED_VALUE_TWO = 23456;
const uint64_t SEED_VALUE_THREE = 34567;
const uint64_t SEED_VALUE_FOUR = 45678;
const uint64_t SEED_VALUE_FIVE = 56789;
const uint64_t SEED_VALUE_SIX = 67890;
const uint64_t SEED_VALUE_SEVEN = 1;
const uint64_t SEED_VALUE_EIGHT = 1023987435908;
const uint64_t SEED_VALUE_NINE = 10923810957;
const uint64_t SEED_VALUE_TEN = 10297834198;
const std::string FILE_PATH = std::string(DUNGEON_ROOMS_DIR) + "/Dungeon_";


TEST_CASE("BSP-Dungeon Constructor", "[core]") { 
    SECTION("Constructor creates a tree of nodes given default parameters") {
        cse498::ForestLevel level;
        cse498::BSP BSP(level, SEED_VALUE_ONE);
        SetDefaultSetting(BSP);
        //Making sure default parameters are created properly   
        CHECK(BSP.GetWidth() == 100);
        CHECK(BSP.GetHeight() == 100);
        CHECK(BSP.GetIterations() == 20);

        //Making sure Nodes are at least created, more detailed testing soon to follow below
        CHECK(BSP.GetBSPTree().size() > 0);
        CHECK(BSP.GetLeafNodes().size() > 0);
    }

    SECTION("Testing that RegenerateObjectState() regenerates BSPTree properly") {
        cse498::ForestLevel level;
        cse498::BSP BSP(level, SEED_VALUE_ONE);
        SetDefaultSetting(BSP);

        BSP.ClearState();

        CHECK(BSP.GetBSPTree().size() == 0);
        CHECK(BSP.GetLeafNodes().size() == 0);

        BSP.CreateBSPTree();

        CHECK(BSP.GetBSPTree().size() != 0);
        CHECK(BSP.GetLeafNodes().size() != 0);


    }


    SECTION("Testing Setting and Getting mWidth/mHeight/Iterations parameters") {
        cse498::ForestLevel level;
        cse498::BSP BSP(level, SEED_VALUE_ONE);
        SetDefaultSetting(BSP);

        ///Checking default parameters
        CHECK(BSP.GetWidth() == 100);
        CHECK(BSP.GetHeight() == 100);
        CHECK(BSP.GetIterations() == 20);

        BSP.SetHeight(160);
        BSP.SetWidth(180);
        BSP.SetIterations(30);

        CHECK(BSP.GetWidth() == 180);
        CHECK(BSP.GetHeight() == 160);
        CHECK(BSP.GetIterations() == 30);       

        BSP.SetHeight(180);
        BSP.SetWidth(210);
        BSP.SetIterations(50);

        CHECK(BSP.GetWidth() == 210);
        CHECK(BSP.GetHeight() == 180);
        CHECK(BSP.GetIterations() == 50);   


    }

}



TEST_CASE("BSP-Dungeon Tree Node Generation", "[core]") { 
    
    SECTION("Testing that the Dungeon Generation parameters (default width/height) are stable with different seeds") { 

        ///Given Default width, height, and iterations
        cse498::ForestLevel level;
        cse498::BSP BSP(level, SEED_VALUE_ONE);
        SetDefaultSetting(BSP);

        CHECK(BSP.GetRngSeed() == SEED_VALUE_ONE);

        //Testing with Seed value 12345
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-12345.md for Tree information
        auto leaf = BSP.GetLeafNodes();
        auto tree = BSP.GetBSPTree();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);


        //Testing with Seed value 23456
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-23456.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_TWO);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_TWO);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 19);
        CHECK(leaf.size() == 10);

        //Testing with Seed value 34567
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-34567.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_THREE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_THREE);
        BSP.RegenerateObjectState();
        
        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 11);
        CHECK(leaf.size() == 6);

        //Testing with Seed value 45678
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-45678.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_FOUR);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_FOUR);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 21);
        CHECK(leaf.size() == 11);

        //Testing with Seed value 56789
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-56789.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_FIVE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_FIVE);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 23);
        CHECK(leaf.size() == 12);

        //Testing with Seed value 67890
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-67890.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_SIX);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_SIX);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 15);
        CHECK(leaf.size() == 8);

        //Testing with Seed value 1 @@@@@
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-1.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_SEVEN);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_SEVEN);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 23);
        CHECK(leaf.size() == 12);

        //Testing with Seed value 1023987435908
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-1023987435908.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_EIGHT);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_EIGHT);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 15);
        CHECK(leaf.size() == 8);

        //Testing with Seed value 10923810957 @@@@@@@@
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-10923810957.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_NINE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_NINE);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 21);
        CHECK(leaf.size() == 11);

        //Testing with Seed value 10317834198
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-10317834198.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_TEN);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_TEN);
        BSP.RegenerateObjectState();

        leaf = BSP.GetLeafNodes();
        tree = BSP.GetBSPTree();

        CHECK(tree.size() == 21);
        CHECK(leaf.size() == 11);

    }
}