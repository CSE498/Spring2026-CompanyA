/**
 * @file TestPathGenerator.cpp
 * @author lrima
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <algorithm>
#include "../../source/tools/PathGenerator.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/WorldGrid.hpp"
#include "../../source/tools/AgentAbility.hpp"
#include "../../source/tools/WorldPath.hpp"

using cse498::WorldPath;
using cse498::PathGenerator;
using cse498::WorldPosition;
using cse498::PathRequest;

namespace cse498
{
class TestWorldBase : public WorldBase
{
protected:
    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };
    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id; ///< Easy access to wall CellType ID.
    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase &agent) override
    {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
    }

public:
    TestWorldBase()
    {
        floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
        wall_id = main_grid.AddCellType("wall", "Impenetrable wall.", '#');
        // NOTE: Top left cell is (0,0) and moving right (x,0) and down (0,y).
    }
    ~TestWorldBase() override = default;
    int DoAction(AgentBase &agent, size_t action_id) override { return 0; }
};

class TestWorld1 : public TestWorldBase
{
public:
    TestWorld1()
    {
        main_grid.Load(std::vector<std::string>{
            "#############################",
            "#           #     #         #", // 1,1
            "# ######### # ### # ####### #",
            "# #       # # #           # #",
            "# # ##### # # # ######### # #",
            "# # #   # #   # #           #",
            "# # # ### ##### # ###########", //6
            "#   # # #       #     #     #",
            "##### # ########### ### ### #",
            "#     #             #   #   #",
            "####### ############# ### ###",
            "#                     #     #", // y=11, x=27
            "#############################"
        });
    }
    ~TestWorld1() override = default;
};
class TestWorld2 : public TestWorldBase
{
public:
    TestWorld2()
    {
        main_grid.Load(std::vector<std::string>{
            "#######################",
            "# #                   #", // (1,1) --> (9, 1)
            "# ###             ### #",
            "# #     #     #  #  # #",
            "# #     #  #  #  #  # #",
            "#          #     #    #",
            "##################  # #",
            "#                    ##",
            "#                    ##",
            "#  ####################",
            "#######################"
        });
    }
    ~TestWorld2() override = default;
};

class TestWorld3 : public TestWorldBase
{
public:
    TestWorld3()
    {
        main_grid.Load(std::vector<std::string>{
            "#####################################",
            "#         #           #             #", // x=3-->13 y=1-->5
            "#         #           #             #",
            "#         ###     #####     ####### #", // y=3, x=13 (2 up, 2 down -- y=1,x=6 (6,1) --> (13,3)
            "#                   #               #", // y=4 x = 19 is wall {19,4} - {1.8}
            "#                   #               #",
            "#####   #########   #####   #########", // row 6, col 19 --> row 9, col 16 // (col 6)
            "#                   #               #",
            "#                   #               #",
            "#   #####     #####   #####     #####",
            "#             #           #         #",
            "#             #           #         #",
            "#####################################"
        });
    }
    ~TestWorld3() override = default;
};

class CircleWorld : public TestWorldBase
{
public:
    CircleWorld()
    {
        main_grid.Load(std::vector<std::string>{
            "####################",
            "#                  #", // {1,1} --> {18}           1,1 --> 2,2 center r2
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "####################",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "#                  #",
            "####################"
        });
    }
    ~CircleWorld() override = default;
};
}

bool WorldPathApprox(const WorldPath &p1, const WorldPath &p2)
{
    if (p1.Size() != p2.Size())
        return false;
    for (size_t i = 0; i < p1.Size(); ++i)
    {
        if (std::abs(p1.At(i).X() - p2.At(i).X()) > 0.001 || std::abs(p1.At(i).Y() - p2.At(i).Y()) > 0.001)
            return false;
    }
    return true;
}

/*
 * Use TEST_CASE, REQUIRE, CHECK, SECTION where appropriate
 *
 */
TEST_CASE("Basic Functionality of the 4 path generators", "[group2]")
{
    /*
     * Shortest Path:
     */
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());

    // Initial Tests to ensure things are working ok -- semi-complex
    auto path = PathGenerator::FindShortestPath({0, 0}, {1, 1}, request2);
    CHECK(!path);
    std::vector<WorldPosition> path2_answer =
    {
        {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {2, 5}, {3, 5}, {4, 4},
        {5, 3}, {5, 2}, {5, 1}, {4, 1}, {3, 1}
    };
    auto path2 = PathGenerator::FindShortestPath({1, 1}, {3, 1}, request2);
    CHECK(WorldPath(path2_answer) == path2.value());

    // Straight line
    auto straight1 = PathGenerator::FindShortestPath({1, 1}, {1, 5}, request2);
    auto straight2 = PathGenerator::FindShortestPath({3, 1}, {8, 1}, request2);
    std::vector<WorldPosition> straight1_answer = {{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}};
    std::vector<WorldPosition> straight2_answer = {{3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}};
    CHECK(WorldPath(straight1_answer) == straight1.value());
    CHECK(WorldPath(straight2_answer) == straight2.value());

    // Diagonal line

    // row 6, col 19 --> row 8, col 17
    auto diagonal1 = PathGenerator::FindShortestPath({19, 6}, {17, 8}, request3);
    // row 6 col 6 2 up 2 down
    auto diagonal2 = PathGenerator::FindShortestPath({4, 8}, {8, 4}, request3);
    std::vector<WorldPosition> diagonal1_answer = {{19, 6}, {18, 7}, {17, 8}};
    std::vector<WorldPosition> diagonal2_answer = {{4, 8}, {5, 7}, {6, 6}, {7, 5}, {8, 4}};
    CHECK(diagonal1.value() == WorldPath(diagonal1_answer));
    CHECK(diagonal2.value() == WorldPath(diagonal2_answer));

    // 2 diagonal lines
    auto diagonal3 = PathGenerator::FindShortestPath({6, 1}, {14, 3}, request3);
    std::vector<WorldPosition> diagonal3_answer = {
        {6, 1}, {7, 2}, {8, 3}, {9, 4}, {10, 4}, {11, 4}, {12, 4}, {13, 4}, {14, 3}
    };
    CHECK(diagonal3.value() == WorldPath(diagonal3_answer));

    // Complex Cases:
    // y=11, x=27

    auto complex1 = PathGenerator::FindShortestPath({1, 1}, {27, 11}, request1);
    std::vector<WorldPosition> complex1_answer = {
        {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {11, 2}, {11, 3},
        {11, 4}, {11, 5}, {12, 5}, {
            13, 5
        },
        {13, 4}, {13, 3}, {13, 2}, {13, 1}, {14, 1}, {15, 1}, {16, 1}, {17, 1}, {17, 2}, {17, 3}, {18, 3}, {19, 3},
        {20, 3}, {21, 3}, {
            22, 3
        },
        {23, 3}, {24, 3}, {25, 3}, {25, 4}, {25, 5}, {24, 5}, {23, 5}, {22, 5}, {21, 5}, {20, 5}, {19, 5}, {18, 5},
        {17, 5}, {17, 6}, {
            17, 7
        },
        {18, 7}, {19, 7}, {19, 8}, {19, 9}, {18, 9}, {17, 9}, {16, 9}, {15, 9}, {14, 9}, {13, 9}, {12, 9}, {11, 9},
        {10, 9}, {9, 9}, {8, 9}, {7, 9}, {7, 10}, {7, 11}, {8, 11}, {9, 11}, {10, 11}, {11, 11}, {12, 11}, {13, 11},
        {14, 11}, {15, 11}, {16, 11}, {17, 11}, {
            18,
            11
        },
        {19, 11}, {20, 11}, {21, 11}, {21, 10}, {21, 9}, {22, 9}, {23, 9}, {23, 8}, {23, 7}, {24, 7}, {25, 7}, {26, 7},
        {27, 7}, {27, 8},
        {27, 9}, {26, 9}, {25, 9}, {25, 10}, {25, 11}, {26, 11}, {27, 11}
    };
    CHECK(complex1.value() == WorldPath(complex1_answer));
    auto complex1_back = PathGenerator::FindShortestPath({27, 11}, {1, 1}, request1);
    std::reverse(complex1_answer.begin(), complex1_answer.end());
    CHECK(complex1_back.value() == WorldPath(complex1_answer));

    /*
     * Manhattan Path: This is relatively simple function so not a lot of options for errors
     */

    // Straight line vertical test
    auto man_simple1 = PathGenerator::FindManhattanPath({1, 1}, {1, 5}, request2);
    std::vector<WorldPosition> man_simple1_answer = {{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}};
    CHECK(man_simple1.value() == WorldPath(man_simple1_answer));

    // Actual manhattan L shape. CW goes --> then down (v) -- FAILURE
    auto man_simple2 = PathGenerator::FindManhattanPath({1, 1}, {15, 5}, request3);
    CHECK(!man_simple2);
    // This goes down then across (v) --> Success
    auto man_simple2_1 = PathGenerator::FindManhattanPath({1, 1}, {15, 5}, request3, cse498::CircleDirectionFlag::CCW);
    std::vector<WorldPosition> man_simple2_1answer = {
        {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5}, {8, 5}, {9, 5}, {10, 5},
        {11, 5}, {12, 5}, {
            13, 5
        },
        {14, 5}, {15, 5}
    };
    CHECK(WorldPath(man_simple2_1answer) == man_simple2_1.value());

    // x=3-->13 y=1-->5
    auto man_different_coord = PathGenerator::FindManhattanPath({3, 1},
                                                                {15, 5},
                                                                request3,
                                                                cse498::CircleDirectionFlag::CCW);
    // Clockwise direction which is reverse of above
    auto man_different_coord_rev = PathGenerator::FindManhattanPath({15, 5}, {3, 1}, request3);
    CHECK(man_different_coord.value() == man_different_coord_rev.value().reverse());

    //{19,4} - {1.8} this is (v) <-- and --> then ^
    auto man_p1 = PathGenerator::FindManhattanPath({19, 4}, {1, 8}, request3);
    auto man_p1_rev = PathGenerator::FindManhattanPath({1, 8}, {19, 4}, request3, cse498::CircleDirectionFlag::CCW);
    CHECK(man_p1.value() == man_p1_rev.value().reverse());

    /*
     * Circle Generation
     */
    cse498::CircleWorld circle_world; // x=[1-18] y=[1,8]
    PathRequest circle_request({}, cse498::AgentAbility(), circle_world.GetGrid());

    // Couple of quick simple tests to ensure it is working in general when pressed into a wall and in free space
    // Edge cases are not tested thoroughly because I could be here for a fortnight.
    auto circ1 = PathGenerator::FindCircularPath({1, 1}, {4, 4}, 2, circle_request);
    std::vector<WorldPosition> circ1_path_to = {{1, 1}, {2, 2}, {2.58579, 2.58579}};
    std::vector<WorldPosition> circle1_path = {
        {2.58579, 2.58579}, {3.44722, 2.07791}, {4.44684, 2.05056}, {5.33476, 2.51056},
        {5.88898, 3.34293}, {5.97096, 4.33956}, {5.5602, 5.25131}, {4.75939, 5.85022},
        {3.76873, 5.98658}, {2.83589, 5.6263}, {2.19408, 4.85944}, {2.00374, 3.87772}, {2.31247, 2.92657}
    };

    CHECK(WorldPathApprox(WorldPath(circ1_path_to), circ1.value().path_to_circle));
    CHECK(WorldPathApprox(WorldPath(circle1_path), circ1.value().circle_path));

    auto circ2 = PathGenerator::FindCircularPath({1, 1}, {2, 2}, 2, circle_request);
    std::vector<WorldPosition> circ2_path_to = {{1, 1}, {2, 1}, {3, 1}, {3.88898, 1.34293}};
    std::vector<WorldPosition> circ2_path = {
        {3.88898, 1.34293}, {3.97096, 2.33956}, {3.5602, 3.25131},
        {2.75939, 3.85022}, {1.76873, 3.98658}, {1.76873, 3.98658}, {2.76873, 2.98658}, {3.76873, 1.98658}
    };

    CHECK(WorldPathApprox(WorldPath(circ2_path_to), circ2.value().path_to_circle));
    CHECK(WorldPathApprox(WorldPath(circ2_path), circ2.value().circle_path));

    // Now Expanding Module -- same same
    // auto circ1_exp = PathGenerator::FindCircularPath({1,1},
    //     {4,4}, 2, circle_request, cse498::PathFlag::Expand);


    auto circ2_exp = PathGenerator::FindCircularPath({1, 1},
                                                     {2, 2},
                                                     2,
                                                     circle_request,
                                                     cse498::PathFlag::Expand);
    std::vector<WorldPosition> circ2_exp_path_to = {{1, 1}, {2, 1}, {3, 1}, {3.88898, 1.34293}};
    std::vector<WorldPosition> circ2_exp_path = {
        {3.88898, 1.34293}, {3.97096, 2.33956}, {3.5602, 3.25131}, {2.75939, 3.85022},
        {1.76873, 3.98658}, {1.76873, 2.98658}, {1.76873, 1.98658}, {2.76873, 1.98658}, {3.76873, 1.98658}
    };
    CHECK(WorldPathApprox(WorldPath(circ2_exp_path), circ2_exp.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(circ2_exp_path_to), circ2_exp.value().path_to_circle));




    /*
     * Now we test the last function. Rectangular loops
     */

    auto rectangle1 = PathGenerator::FindRectangularLoopPath({1, 1}, {2.5, 5}, {5.5, 1}, circle_request);
    std::vector<WorldPosition> rectangle1_path_to = {{1, 1}, {2, 1}, {2.5, 1}};
    std::vector<WorldPosition> rectangle1_path = {
        {2.5, 1}, {3.5, 1}, {4.5, 1}, {5.5, 1}, {5.5, 2}, {5.5, 3}, {5.5, 4}, {5.5, 5}, {4.5, 5}, {3.5, 5}, {2.5, 5},
        {2.5, 4}, {2.5, 3}, {2.5, 2}
    };
    CHECK(WorldPathApprox(WorldPath(rectangle1_path), rectangle1.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(rectangle1_path_to), rectangle1.value().path_to_circle));

    // Opposite diretion works as well
    auto rectangle2 = PathGenerator::FindRectangularLoopPath({1, 1},
                                                             {2.5, 5},
                                                             {5.5, 1},
                                                             circle_request,
                                                             cse498::CircleDirectionFlag::CCW);
    std::vector<WorldPosition> rectangle2_path_to = {{1, 1}, {2, 1}, {2.5, 1}};
    std::vector<WorldPosition> rectangle2_path = {
        {2.5, 1}, {2.5, 2}, {2.5, 3}, {2.5, 4}, {2.5, 5}, {3.5, 5}, {4.5, 5}, {5.5, 5}, {5.5, 4}, {5.5, 3}, {5.5, 2},
        {5.5, 1}, {4.5, 1}, {3.5, 1}
    };
    CHECK(WorldPathApprox(WorldPath(rectangle2_path), rectangle2.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(rectangle2_path_to), rectangle2.value().path_to_circle));





    // TODO: Need to test backwards CCW CW flags on everything.
    // TODO test opposite direction generation
}

TEST_CASE("Path Generation Edge Cases", "[group2]")
{
    // test invalid position to valid
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());

    // invalid position to invalid position
    auto in2in = PathGenerator::FindShortestPath({0, 2}, {0, 0}, request2);
    CHECK(!in2in);

    // valid to invalid
    auto v2in = PathGenerator::FindShortestPath({1, 2}, {0, 0}, request2);
    CHECK(!v2in);

    // valid to valid (same position)
    auto v2v = PathGenerator::FindShortestPath({1, 2}, {1, 2}, request2);
    std::vector<WorldPosition> v2v_answer = {{1, 2}};
    CHECK(v2v.value() == WorldPath(v2v_answer));

    // Trapped and can't move case
    auto trapped = PathGenerator::FindShortestPath({1, 9}, {1, 1}, request1);
    CHECK(!trapped);

    auto man_p1 = PathGenerator::FindManhattanPath({1, 1}, {1, 1}, request1);
    std::vector<WorldPosition> man_p1_ans = {{1, 1}};
    CHECK(man_p1.value() == WorldPath(man_p1_ans));

    auto man_p1_inv = PathGenerator::FindManhattanPath({0, 1}, {1, 1}, request1);
    CHECK(!man_p1_inv);
}

TEST_CASE("Obstacles", "[group2]")
{
    // Avoid tiles making a trapped box
    // Skipped due to this already being too much to do in too little time.
    // Will flesh this out later if this all sticks around.
}

TEST_CASE("Path Generation - Doubles testing", "[group2]")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());
    cse498::CircleWorld circle_world; // x=[1-18] y=[1,8]
    PathRequest circle_request({}, cse498::AgentAbility(), circle_world.GetGrid());

    // Initial Tests to ensure things are working ok -- semi-complex
    //  (1,1) --> (9, 1)
    auto path = PathGenerator::FindShortestPath({1.5, 1.3}, {9.32, 1.35}, request2);
    std::vector<WorldPosition> path1 = {
        {1.5, 1.3}, {1.5, 2.3}, {1.5, 3.3}, {1.5, 4.3}, {1.5, 5.3}, {2.5, 5.3}, {3.5, 5.3}, {4.5, 4.3}, {5.5, 3.3},
        {6.5, 3.3}, {7.5, 2.3},
        {8.5, 2.3}, {9.5, 1.3}
    };
    CHECK(WorldPath(path1) == path.value());
    auto path2 = PathGenerator::FindShortestPath({0.5, 1.3}, {9.32, 1.35}, request2);
    CHECK(!path2);

    auto circle1 = PathGenerator::FindCircularPath({1, 1}, {3.52, 3.32}, 4, circle_request);
    std::vector<WorldPosition> circle1_path_to = {
        {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {7.21621, 1.79093}
    };
    std::vector<WorldPosition> circle1_path = {
        {7.21621, 1.79093}, {7.47997, 2.75552}, {7.49623, 3.75539}, {7.26398, 4.72804}, {6.79773, 5.6127},
        {6.12663, 6.35406}, {5.2926, 6.90579}, {4.34779, 7.23341}, {3.35125, 7.31644}, {2.36525, 7.14969},
        {1.45142, 6.74359}, {2.45142, 5.74359}, {3.45142, 4.74359}, {4.45142, 3.74359}, {5.45142, 3.74359},
        {6.45142, 2.74359}, {7.45142, 1.74359}
    };
    CHECK(WorldPathApprox(WorldPath(circle1_path_to), circle1.value().path_to_circle));
    CHECK(WorldPathApprox(WorldPath(circle1_path), circle1.value().circle_path));

    auto circle1_exp = PathGenerator::FindCircularPath({1, 1},
                                                       {3.52, 3.32},
                                                       4,
                                                       circle_request,
                                                       cse498::PathFlag::Expand);
    std::vector<WorldPosition> circle1_exp_path_to = {
        {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {7.21621, 1.79093}
    };
    std::vector<WorldPosition> circle1_exp_path = {
        {7.21621, 1.79093}, {7.47997, 2.75552}, {7.49623, 3.75539}, {7.26398, 4.72804}, {6.79773, 5.6127},
        {6.12663, 6.35406}, {5.2926, 6.90579}, {4.34779, 7.23341}, {3.35125, 7.31644}, {2.36525, 7.14969},
        {1.45142, 6.74359}, {1.45142, 5.74359}, {1.45142, 4.74359}, {1.45142, 3.74359}, {1.45142, 2.74359},
        {1.45142, 1.74359}, {2.45142, 1.74359}, {3.45142, 1.74359}, {4.45142, 1.74359}, {5.45142, 1.74359},
        {6.45142, 1.74359}
    };
    CHECK(WorldPathApprox(WorldPath(circle1_exp_path_to), circle1_exp.value().path_to_circle));
    CHECK(WorldPathApprox(WorldPath(circle1_exp_path), circle1_exp.value().circle_path));


    // TODO check circle generation against walls
}


TEST_CASE("Path Generation -- Parts of path outside of area")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());
    cse498::CircleWorld circle_world; // x=[1-18] y=[1,8]
    PathRequest circle_request({}, cse498::AgentAbility(), circle_world.GetGrid());

    // 1. Shortest Path
    auto path1 = PathGenerator::FindShortestPath({0.5, 1.3}, {9.32, -3.35}, request2);
    auto path2 = PathGenerator::FindShortestPath({-3.5, 1.3}, {9.32, 1.35}, request2);
    CHECK(!path1);
    CHECK(!path2);

    // 2. Circle Path
    auto circle1 = PathGenerator::FindCircularPath({1,1}, {-4,4}, 3, circle_request);
    CHECK(!circle1);

    // 3. Loop Path
    // This hits the assert statement and fails as expected because this isn't permitted behavior of the function
    // auto loop1 = PathGenerator::FindRectangularLoopPath({1,1}, {-4,5}, {4,4}, request1);
    // CHECK(!loop1);

    // 4. Manhattan paths
    auto man1 = PathGenerator::FindManhattanPath({1,-1}, {-4,4}, request1);
    CHECK(!man1);

}

// test case - shortest path with obstacles.