#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../group_specific_content/Group-15/WorldGrid.hpp"
#include "../../group_specific_content/Group-15/Location.hpp"

#include <sstream>
#include <string>
#include <vector>



void AddBasicCellTypes(cse498::WorldGrid& grid) {
  // ID 0 is "Unknown" reserved by constructor.
  grid.AddCellType("Wall", "Solid wall", '#');
  grid.AddCellType("Floor", "Walkable floor", '.');
  grid.AddCellType("Door", "Door", 'D');
}

cse498::WorldGrid MakeSmallGrid3x2() {
  cse498::WorldGrid grid(3, 2);
  AddBasicCellTypes(grid);
  return grid;
}


TEST_CASE("WorldGrid Constructor", "[core]") {
  SECTION("Default constructor yields empty grid, but has Unknown type") {
    cse498::WorldGrid grid;

    CHECK(grid.GetWidth() == 0);
    CHECK(grid.GetHeight() == 0);
    CHECK(grid.GetNumCells() == 0);

    // Unknown type exists at ID 0
    CHECK(grid.GetCellTypes().size() >= 1);
    CHECK(grid.GetCellTypeName(0) == "Unknown");
  }

  SECTION("Sized constructor sets width/height and correct cell count") {
    cse498::WorldGrid grid(4, 3);

    CHECK(grid.GetWidth() == 4);
    CHECK(grid.GetHeight() == 3);
    CHECK(grid.GetNumCells() == 12);

    // All default to 0 initially
    CHECK(grid[std::pair<size_t,size_t>{0,0}] == 0);
    CHECK(grid[std::pair<size_t,size_t>{3,2}] == 0);
  }
}

TEST_CASE("IsValid Bounds Checking", "[core]") {
  cse498::WorldGrid grid(4, 3);

  SECTION("Valid corners") {
    CHECK(grid.IsValid(0, 0));
    CHECK(grid.IsValid(3, 2));
  }

  SECTION("Invalid negatives") {
    CHECK_FALSE(grid.IsValid(-1, 0));
    CHECK_FALSE(grid.IsValid(0, -1));
  }

  SECTION("Invalid too large") {
    CHECK_FALSE(grid.IsValid(4, 0)); // x out of range
    CHECK_FALSE(grid.IsValid(0, 3)); // y out of range
  }
}
