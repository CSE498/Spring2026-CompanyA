//#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/WorldPosition.hpp"

TEST_CASE("Test WorldPosition Constructor", "[core]")
{
    SECTION("Default Constructor")
    {
        cse498::WorldPosition pos;
        CHECK(pos.X() == 0.0);
        CHECK(pos.Y() == 0.0);
    }

    SECTION("Constructor with doubles")
    {
        cse498::WorldPosition pos2(3.5, 4.5);
        CHECK(pos2.X() == 3.5);
        CHECK(pos2.Y() == 4.5);
    }

    SECTION("Constructor with integers")
    {
        cse498::WorldPosition pos3(3, 4);
        CHECK(pos3.X() == 3.0);
        CHECK(pos3.Y() == 4.0);
    }

}

TEST_CASE("Test Getdir and Setdir", "[core]")
{
    SECTION("Default direction is NORTH") 
    {
        cse498::WorldPosition pos(5.0, 10.0);
        CHECK(pos.GetDir() == cse498::WorldPosition::Orientation::NORTH);
    }
    
    SECTION("Set direction to SOUTH") 
    {
        cse498::WorldPosition pos(5.0, 10.0);
        pos.SetDir(cse498::WorldPosition::Orientation::SOUTH);
        CHECK(pos.GetDir() == cse498::WorldPosition::Orientation::SOUTH);
    }
    
    SECTION("Change direction multiple times") 
    {
        cse498::WorldPosition pos(5.0, 10.0);
        pos.SetDir(cse498::WorldPosition::Orientation::EAST);
        pos.SetDir(cse498::WorldPosition::Orientation::WEST);
        CHECK(pos.GetDir() == cse498::WorldPosition::Orientation::WEST);
    }
}

TEST_CASE("Test isColliding", "[core]") {
    
    SECTION("Same position") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 10.0);
        CHECK(pos1.isColliding(pos2) == true);
    }
    
    SECTION("Same cell however different decimals") {
        cse498::WorldPosition pos1(5.1, 10.2);
        cse498::WorldPosition pos2(5.9, 10.8);
        CHECK(pos1.isColliding(pos2) == true);
    }
    
    SECTION("Different cells") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(6.0, 10.0);
        CHECK(pos1.isColliding(pos2) == false);
    }
    
    SECTION("Adjacent cells") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 11.0);
        CHECK(pos1.isColliding(pos2) == false);
    }
}

TEST_CASE("Test IsAdjacentCell", "[core]") {
    
    SECTION("Same cell") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }
    
    SECTION("One cell to the right") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(6.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell to the left") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(4.0, 10.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell up") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 9.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }

    SECTION("One cell down") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 11.0);
        CHECK(pos1.IsAdjacentCell(pos2) == true);
    }
    
    SECTION("Diagonal") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(6.0, 11.0);
        CHECK(pos1.IsAdjacentCell(pos2) == false);
    }
}

TEST_CASE("Test CellDistance", "[core]") {
    
    SECTION("Same cell") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(5.0, 10.0);
        CHECK(pos1.CellDistance(pos2) == 0);
    }
    
    SECTION("One cell away") {
        cse498::WorldPosition pos1(5.0, 10.0);
        cse498::WorldPosition pos2(6.0, 10.0);
        CHECK(pos1.CellDistance(pos2) == 1);
    }

    SECTION("Far apart") {
        cse498::WorldPosition pos1(0.0, 10.0);
        cse498::WorldPosition pos2(6.0, 11.0);
        CHECK(pos1.CellDistance(pos2) == 7);
    }

    SECTION("Very far apart") {
        cse498::WorldPosition pos1(0.0, 1.0);
        cse498::WorldPosition pos2(20.0, 25.0);
        CHECK(pos1.CellDistance(pos2) == 44);
    }
}