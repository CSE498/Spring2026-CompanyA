
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/core/BSP-Dungeon.hpp"

/// @brief Brought over from DungeonOne for test case purposes
/// @return vector of 
static cse498::WeightedSet<std::string> MakeRoomPool() {
    cse498::WeightedSet<std::string> rooms;
    std::string file_name = "one_pool/room_";
    rooms.Insert(file_name + "1.txt", 10);
    rooms.Insert(file_name + "2.txt", 10);
    rooms.Insert(file_name + "3.txt", 10);
    rooms.Insert(file_name + "4.txt", 10);
    rooms.Insert(file_name + "5.txt", 10);
    rooms.Insert(file_name + "6.txt", 1);
    return rooms;
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
const std::string FILE_PATH = "../../source/core/rooms/Dungeon_";


TEST_CASE("BSP-Dungeon Constructor", "[core]") { 
    SECTION("Constructor creates a tree of nodes given default parameters") {
        cse498::BSP BSP(MakeRoomPool(), SEED_VALUE_ONE, FILE_PATH);
        
        //Making sure default parameters are created properly
        CHECK(BSP.GetWidth() == 150);
        CHECK(BSP.GetHeight() == 100);
        CHECK(BSP.GetIterations() == 20);

        //Making sure Nodes are at least created, more detailed testing soon to follow below
        CHECK(BSP.GetBSPTree().size() > 0);
        CHECK(BSP.GetLeafNodes().size() > 0);
    }

    SECTION("Testing that RegenerateObjectState() regenerates BSPTree properly") {
        cse498::BSP BSP(MakeRoomPool(), SEED_VALUE_ONE, FILE_PATH);

        BSP.ClearState();

        CHECK(BSP.GetBSPTree().size() == 0);
        CHECK(BSP.GetLeafNodes().size() == 0);

        BSP.RepopulateTree();

        CHECK(BSP.GetBSPTree().size() != 0);
        CHECK(BSP.GetLeafNodes().size() != 0);


    }


    SECTION("Testing Setting and Getting mWidth/mHeight/Iterations parameters") {
        cse498::BSP BSP(MakeRoomPool(), SEED_VALUE_ONE, FILE_PATH);
        ///Checking default parameters
        CHECK(BSP.GetWidth() == 150);
        CHECK(BSP.GetHeight() == 100);
        CHECK(BSP.GetIterations() == 20);

        BSP.SetHeight(150);
        BSP.SetWidth(180);
        BSP.SetIterations(30);

        CHECK(BSP.GetWidth() == 180);
        CHECK(BSP.GetHeight() == 150);
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
        /**
         * This section here uses default width/height parameters, meaning that the size of the tree should remain the same and stable,
         * even with different seed inputs
        */

        ///Given Default width, height, and iterations
        cse498::BSP BSP(MakeRoomPool(), SEED_VALUE_ONE, FILE_PATH);
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
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 34567
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-34567.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_THREE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_THREE);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 45678
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-45678.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_FOUR);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_FOUR);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 56789
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-56789.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_FIVE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_FIVE);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 67890
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-67890.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_SIX);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_SIX);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 1 @@@@@
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-1.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_SEVEN);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_SEVEN);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 1023987435908
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-1023987435908.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_EIGHT);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_EIGHT);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 10923810957 @@@@@@@@
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-10923810957.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_NINE);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_NINE);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

        //Testing with Seed value 10297834198
        //Refer to BSP-Dungeon-Seed-Info/BSP-Dungeon-Seed-10297834198.md for Tree information
        BSP.SetRngSeed(SEED_VALUE_TEN);
        CHECK(BSP.GetRngSeed() == SEED_VALUE_TEN);
        BSP.RegnerateObjectState();
        CHECK(tree.size() == 29);
        CHECK(leaf.size() == 15);

    }
}