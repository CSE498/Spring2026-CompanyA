/**
 * @file TestPathGenerator.cpp
 * @author Logan
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
using std::vector;

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
    int DoAction([[maybe_unused]] AgentBase &agent, [[maybe_unused]] size_t action_id) override { return 0; }
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

class CircleWorld2 : public TestWorldBase
{
public:
    CircleWorld2()
    {
        // (9,-3), (10,-3), (11,-3), (12,-3), (9,-4), (10,-4), (11,-4), (12,-4)
        // (15,-6), (16,-6), (17,-6), (18,-6), (15,-7), (16,-7), (17,-7), (18,-7)

        main_grid.Load(std::vector<std::string>{
            "############################", // Dimensions: x: [1,26] y: [1,15]
            "#                          #",
            "#                          #",
            "#        ####              #", // x=9 first block
            "#        ####              #",
            "#                          #",
            "#              ####        #",
            "#              ####        #",
            "#                          #",
            "#                          #",
            "#   ###########            #", // y=10
            "#   #                     ##",
            "#   #                      #",
            "#   ###########            #",
            "#                          #",
            "#                          #",
            "############################"
        });
    }
    ~CircleWorld2() override = default;
};
}

/**
 * checks points are approximately equal in a world path
 * @param p1 - point 1
 * @param p2 - point 2
 * @return t/f -- true if all points in path are approx equal
 */
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

TEST_CASE("No Constructor", "[PathGenerator, Constructor]")
{
    CHECK(!std::is_constructible<PathGenerator>::value);
    CHECK(!std::is_copy_constructible<PathGenerator>::value);
    CHECK(!std::is_move_constructible<PathGenerator>::value);
}

TEST_CASE("Shortest Path Generation -- Simple cases for functionality", "[ShorestPath]")
{
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
    // CHECK reverse case of the complex case: easy extra test
    CHECK(complex1_back.value() == WorldPath(complex1_answer));
}

TEST_CASE("Shortest Path Generation -- Edge Cases", "[ShortestPath]")
{
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());

    // 0 to 0
    auto none2none = PathGenerator::FindShortestPath({1, 1}, {1, 1}, request1);
    vector<WorldPosition> none2none_answer = {{1, 1}};
    CHECK(none2none.value() == WorldPath(none2none_answer));

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
}

TEST_CASE("Shortest Path Generation -- Obstacles", "[ShortestPath]")
{
    cse498::TestWorld2 world2;
    std::unordered_set<WorldPosition> tiles_to_avoid = {{1, 2}};
    PathRequest request2_1(tiles_to_avoid, cse498::AgentAbility(), world2.GetGrid());

    auto trapped = PathGenerator::FindShortestPath({1, 1}, {1, 5}, request2_1);
    CHECK(!trapped);

    // edge case
    auto trapped2 = PathGenerator::FindShortestPath({1, 2}, {1, 2}, request2_1);
    CHECK(!trapped);

    // Genuinely there isn't much more to test with tiles to avoid because this ensures that they are treated
    // as walls and this is how it was made so. It is a wall. Good.
}

TEST_CASE("Circular Path Generation -- Simple Cases", "[CirclePath]")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());
    cse498::CircleWorld circle_world;
    PathRequest circle_request({}, cse498::AgentAbility(), circle_world.GetGrid());

    // Couple of quick simple tests to ensure it is working in general when pressed into a wall and in free space
    // Edge cases are not tested thoroughly because I could be here for a fortnight.
    auto circ1 = PathGenerator::FindCircularPath({1, 1}, {4, 4}, 2, circle_request);
    std::vector<WorldPosition> circ1_path_to = {{1, 1}, {2.58579, 2.58579}};
    std::vector<WorldPosition> circle1_path = {
        {2.58579, 2.58579}, {3.44722, 2.07791}, {4.44684, 2.05056}, {5.33476, 2.51056}, {5.88898, 3.34293},
        {5.97096, 4.33956}, {5.5602, 5.25131}, {4.75939, 5.85022}, {3.76873, 5.98658}, {2.83589, 5.6263},
        {2.19408, 4.85944}, {2.00374, 3.87772}, {2.31247, 2.92657}
    };

    CHECK(WorldPathApprox(WorldPath(circ1_path_to), circ1.value().path_to_circle));
    CHECK(WorldPathApprox(WorldPath(circle1_path), circ1.value().circle_path));

    auto circ2 = PathGenerator::FindCircularPath({1, 1}, {2, 2}, 2, circle_request);
    std::vector<WorldPosition> circ2_path_to = {{1, 1}, {2, 1}, {3.88898, 1.34293}};
    std::vector<WorldPosition> circ2_path = {
        {3.88898, 1.34293}, {3.97096, 2.33956}, {3.5602, 3.25131}, {2.75939, 3.85022}, {1.76873, 3.98658},
        {1.76873, 3.98658}, {2.76873, 2.98658}
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
    std::vector<WorldPosition> circ2_exp_path_to = {{1, 1}, {2, 1}, {3.88898, 1.34293}};
    std::vector<WorldPosition> circ2_exp_path = {
        {3.88898, 1.34293}, {3.97096, 2.33956}, {3.5602, 3.25131}, {2.75939, 3.85022}, {1.76873, 3.98658},
        {1.76873, 2.98658}, {1.76873, 1.98658}, {2.76873, 1.98658}, {3.76873, 1.98658}
    };
    CHECK(WorldPathApprox(WorldPath(circ2_exp_path), circ2_exp.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(circ2_exp_path_to), circ2_exp.value().path_to_circle));

    cse498::CircleWorld2 circle_world2;
    PathRequest circle_request2({}, cse498::AgentAbility(), circle_world2.GetGrid());

    // Generation with center on wall
    // FOREWARNING: Looking at this in desmos can be confusing it can look like "Why did it take that path??"
    // NOTE: Desmos Points Imagine DOWN TO RIGHT 1x1 BLOCK consumed from a point.
    // NOTE 2: We move in 1 unit steps so if it is left of grid lines in desmos then that is correct position
    auto path1 = PathGenerator::FindCircularPath({1, 1}, {9, 3}, 4, circle_request2);
    vector<WorldPosition> circle_path1 = {
        {5.11943, 2.02986}, {5.48133, 1.09764}, {6.48133, 1.09764}, {7.48133, 1.09764}, {8.48133, 1.09764},
        {9.48133, 1.09764}, {10.4813, 1.09764}, {11.4813, 1.09764}, {12.6995, 1.47903}, {12.9612, 2.4442},
        {13.9612, 2.4442}, {13.9612, 3.4442}, {13.9612, 4.4442}, {13.9612, 5.4442}, {12.2727, 5.29987}, {11.6, 6.03976},
        {10.7647, 6.58966}, {9.81922, 6.91521}, {8.82249, 6.99606}, {7.83686, 6.82715}, {6.92392, 6.41905},
        {6.14074, 5.79726}, {5.53627, 5.00063}, {5.14827, 4.07897}, {5.00101, 3.08988}
    };
    vector<WorldPosition> circle_path1_to = {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5.11943, 2.02986}};
    CHECK(WorldPathApprox(WorldPath(circle_path1), path1.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(circle_path1_to), path1.value().path_to_circle));

    auto path2 = PathGenerator::FindCircularPath({15, 3}, {10, 4}, 7, circle_request2);
    std::vector<WorldPosition> path2_circle = {
        {16.8641, 2.62719}, {16.9896, 3.61927}, {16.9726, 4.61913}, {16.8132, 5.60634}, {15.8132, 5.60634},
        {14.8132, 5.60634}, {14.8132, 6.60634}, {14.8132, 7.60634}, {14.8132, 8.60634}, {14.8596, 9.03832},
        {14.0921, 9.67936}, {13.0921, 9.67936}, {12.0921, 9.67936}, {11.0921, 9.67936}, {10.0921, 9.67936},
        {9.09205, 9.67936}, {8.09205, 9.67936}, {7.09205, 9.67936}, {6.09205, 9.67936}, {5.69465, 9.51942},
        {4.95211, 8.84962}, {4.31258, 8.08085}, {3.78913, 7.22879}, {3.39243, 6.31084}, {3.13058, 5.34574},
        {3.00891, 4.35316}, {3.02993, 3.35339}, {3.19319, 2.3668}, {3.49536, 1.41355}, {4.49536, 1.41355},
        {5.49536, 1.41355}, {6.49536, 1.41355}, {7.49536, 1.41355}, {8.49536, 1.41355}, {9.49536, 1.41355},
        {10.4954, 1.41355}, {11.4954, 1.41355}, {12.4954, 1.41355}, {13.4954, 1.41355}, {14.4954, 1.41355},
        {15.4954, 1.41355}, {16.6166, 1.71523}
    };
    std::vector<WorldPosition> path2_circle_to = {{15, 3}, {16.8641, 2.62719}};
    CHECK(WorldPathApprox(WorldPath(path2_circle), path2.value().circle_path));
    CHECK(WorldPathApprox(WorldPath(path2_circle_to), path2.value().path_to_circle));

    auto path2_exp = PathGenerator::FindCircularPath({15, 3},
                                                     {10, 4},
                                                     8,
                                                     circle_request2,
                                                     cse498::PathFlag::Expand);
    CHECK(path2_exp); // idc about the result it is long.

}

TEST_CASE("Circular Path Generation -- Edge Cases", "[CirclePath]")
{

}

TEST_CASE("Manhattan Path Generation -- Simple Cases", "[ManhattanPath]")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());

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
}

TEST_CASE("Manhattan Path Generation -- Edge Cases", "[ManhattanPath]")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());

    auto man_p1 = PathGenerator::FindManhattanPath({1, 1}, {1, 1}, request1);
    std::vector<WorldPosition> man_p1_ans = {{1, 1}};
    CHECK(man_p1.value() == WorldPath(man_p1_ans));

    auto man_p1_inv = PathGenerator::FindManhattanPath({0, 1}, {1, 1}, request1);
    CHECK(!man_p1_inv);
}

TEST_CASE("Rectangular Loop Generation -- Simple Cases", "[RectangularLoop]")
{
    cse498::TestWorld1 world1;
    PathRequest request1({}, cse498::AgentAbility(), world1.GetGrid());
    cse498::TestWorld2 world2;
    PathRequest request2({}, cse498::AgentAbility(), world2.GetGrid());
    cse498::TestWorld3 world3;
    PathRequest request3({}, cse498::AgentAbility(), world3.GetGrid());
    cse498::CircleWorld circle_world;
    PathRequest circle_request({}, cse498::AgentAbility(), circle_world.GetGrid());

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
}

TEST_CASE("Rectangular Loop Generation -- Edge Cases", "[RectangularLoop]")
{
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
        {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7.21621, 1.79093}
    };
    std::vector<WorldPosition> circle1_path = {
        {7.21621, 1.79093}, {7.47997, 2.75552}, {7.49623, 3.75539}, {7.26398, 4.72804}, {6.79773, 5.6127},
        {6.12663, 6.35406}, {5.2926, 6.90579}, {4.34779, 7.23341}, {3.35125, 7.31644}, {2.36525, 7.14969},
        {1.45142, 6.74359}, {2.45142, 5.74359}, {3.45142, 4.74359}, {4.45142, 3.74359}, {5.45142, 3.74359},
        {6.45142, 2.74359}
    };

    auto circle1_exp = PathGenerator::FindCircularPath({1, 1},
                                                       {3.52, 3.32},
                                                       4,
                                                       circle_request,
                                                       cse498::PathFlag::Expand);
    std::vector<WorldPosition> circle1_exp_path_to = {
        {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7.21621, 1.79093}
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

TEST_CASE("Path Generation ALL - testing paths outside of bounds")
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
    auto circle1 = PathGenerator::FindCircularPath({1, 1}, {-4, 4}, 3, circle_request);
    CHECK(!circle1);

    // 3. Loop Path
    // This hits the assert statement and fails as expected because this isn't permitted behavior of the function
    // auto loop1 = PathGenerator::FindRectangularLoopPath({1,1}, {-4,5}, {4,4}, request1);
    // CHECK(!loop1);

    // 4. Manhattan paths
    auto man1 = PathGenerator::FindManhattanPath({1, -1}, {-4, 4}, request1);
    CHECK(!man1);
}
