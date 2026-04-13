#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/tools/WorldGridPosition.hpp"

TEST_CASE("Test WorldGridPosition Constructor", "[core]")
{
    SECTION("Default Constructor")
    {
        cse498::WorldGridPosition pos;
        CHECK(pos.X() == 0.0);
        CHECK(pos.Y() == 0.0);
    }

    SECTION("Constructor with doubles")
    {
        cse498::WorldGridPosition pos2(3.5, 4.5);
        CHECK(pos2.X() == 3.5);
        CHECK(pos2.Y() == 4.5);
    }

    SECTION("Constructor with integers")
    {
        cse498::WorldGridPosition pos3(3, 4);
        CHECK(pos3.X() == 3.0);
        CHECK(pos3.Y() == 4.0);
    }

}

TEST_CASE("Test Getdir and Setdir", "[core]")
{
    SECTION("Default direction is NORTH") 
    {
        cse498::WorldGridPosition pos(5.0, 10.0);
        CHECK(pos.Dir() == cse498::WorldGridPosition::Orientation::NORTH);
    }
    
    SECTION("Set direction to SOUTH") 
    {
        cse498::WorldGridPosition pos(5.0, 10.0);
        pos.SetDir(cse498::WorldGridPosition::Orientation::SOUTH);
        CHECK(pos.Dir() == cse498::WorldGridPosition::Orientation::SOUTH);
    }
    
    SECTION("Change direction multiple times") 
    {
        cse498::WorldGridPosition pos(5.0, 10.0);
        pos.SetDir(cse498::WorldGridPosition::Orientation::EAST);
        pos.SetDir(cse498::WorldGridPosition::Orientation::WEST);
        CHECK(pos.Dir() == cse498::WorldGridPosition::Orientation::WEST);
    }
}

TEST_CASE("Test isColliding", "[core]") {
    
    SECTION("Same position") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 10.0);
        CHECK(pos1.IsColliding(pos2) == true);
    }
    
    SECTION("Same cell however different decimals") {
        cse498::WorldGridPosition pos1(5.1, 10.2);
        cse498::WorldGridPosition pos2(5.9, 10.8);
        CHECK(pos1.IsColliding(pos2) == true);
    }
    
    SECTION("Different cells") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 10.0);
        CHECK(pos1.IsColliding(pos2) == false);
    }
    
    SECTION("Adjacent cells") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 11.0);
        CHECK(pos1.IsColliding(pos2) == false);
    }
}

TEST_CASE("Test IsAdjacentCell", "[core]") {
    
    SECTION("Same cell") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }
    
    SECTION("One cell to the right") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell to the left") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(4.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell up") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 9.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell down") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 11.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }
    
    SECTION("Diagonal") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 11.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }
}

TEST_CASE("Test CellDistance", "[core]") {
    
    SECTION("Same cell") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 10.0);
        CHECK(pos1.CellDistance(pos2) == 0);
    }
    
    SECTION("One cell away") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 10.0);
        CHECK(pos1.CellDistance(pos2) == 1);
    }

    SECTION("Far apart") {
        cse498::WorldGridPosition pos1(0.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 11.0);
        CHECK(pos1.CellDistance(pos2) == 7);
    }

    SECTION("Very far apart") {
        cse498::WorldGridPosition pos1(0.0, 1.0);
        cse498::WorldGridPosition pos2(20.0, 25.0);
        CHECK(pos1.CellDistance(pos2) == 44);
    }
}

TEST_CASE("Test Offset", "[core]") {

    SECTION("Positive offset") {
        cse498::WorldGridPosition pos(5.0, 10.0);
        cse498::WorldGridPosition result = pos.Offset(2.0, 3.0);
        CHECK(result.X() == 7.0);
        CHECK(result.Y() == 13.0);
    }

    SECTION("Negative offset") {
        cse498::WorldGridPosition pos(5.0, 10.0);
        cse498::WorldGridPosition result = pos.Offset(-2.0, -3.0);
        CHECK(result.X() == 3.0);
        CHECK(result.Y() == 7.0);
    }

    SECTION("Zero offset returns same position") {
        cse498::WorldGridPosition pos(5.0, 10.0);
        cse498::WorldGridPosition result = pos.Offset(0.0, 0.0);
        CHECK(result.X() == 5.0);
        CHECK(result.Y() == 10.0);
    }


    SECTION("Fractional offset") {
        cse498::WorldGridPosition pos(1.0, 1.0);
        cse498::WorldGridPosition result = pos.Offset(0.5, 0.5);
        CHECK(result.X() == 1.5);
        CHECK(result.Y() == 1.5);
    }
}

TEST_CASE("Test CellDistance edge cases", "[core]") {

    SECTION("Diagonal distance") {
        cse498::WorldGridPosition pos1(0.0, 0.0);
        cse498::WorldGridPosition pos2(3.0, 4.0);
        CHECK(pos1.CellDistance(pos2) == 7);
    }

    SECTION("Distance is symmetric") {
        cse498::WorldGridPosition pos1(2.0, 5.0);
        cse498::WorldGridPosition pos2(8.0, 1.0);
        CHECK(pos1.CellDistance(pos2) == pos2.CellDistance(pos1));
    }

    SECTION("Distance from origin") {
        cse498::WorldGridPosition pos1(0.0, 0.0);
        cse498::WorldGridPosition pos2(10.0, 10.0);
        CHECK(pos1.CellDistance(pos2) == 20);
    }
}

TEST_CASE("Test IsAdjacentCell edge cases", "[core]") {

    SECTION("Two cells apart horizontally is not adjacent") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(7.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }

    SECTION("Two cells apart vertically is not adjacent") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.0, 12.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }

    SECTION("Adjacency is symmetric") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == pos2.IsAdjacentCell(pos1));
    }
}

TEST_CASE("Test IsColliding edge cases", "[core]") {

    SECTION("Collision is symmetric") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(5.5, 10.5);
        CHECK(pos1.IsColliding(pos2) == pos2.IsColliding(pos1));
    }

    SECTION("Position at cell boundary does not collide with next cell") {
        cse498::WorldGridPosition pos1(5.0, 10.0);
        cse498::WorldGridPosition pos2(6.0, 10.0);
        CHECK(pos1.IsColliding(pos2) == false);
    }
}