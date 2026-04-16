//#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/core/WorldGrid.hpp"
#include "../../../source/core/Location.hpp"

#include <sstream>
#include <string>
#include <vector>


/*void AddBasicCellTypes(cse498::WorldGrid& grid) {
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
    //CHECK(grid[std::pair<size_t,size_t>{0,0}] == 0);
    //CHECK(grid[std::pair<size_t,size_t>{3,2}] == 0);
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


TEST_CASE("CellType registration and lookup", "[core]") {
  cse498::WorldGrid grid;

  // Unknown is already present at ID 0
  REQUIRE(grid.GetCellTypes().size() == 1);
  CHECK(grid.GetCellTypeName(0) == "Unknown");

  SECTION("AddCellType returns increasing IDs and GetCellTypeName works") {
    size_t wall_id  = grid.AddCellType("Wall",  "Solid wall", '#');
    size_t floor_id = grid.AddCellType("Floor", "Walkable floor", '.');
    size_t door_id  = grid.AddCellType("Door",  "Door", 'D');

    CHECK(wall_id  == 1);
    CHECK(floor_id == 2);
    CHECK(door_id  == 3);

    CHECK(grid.GetCellTypeName(wall_id)  == "Wall");
    CHECK(grid.GetCellTypeName(floor_id) == "Floor");
    CHECK(grid.GetCellTypeName(door_id)  == "Door");

    CHECK(grid.GetCellTypeSymbol(wall_id)  == '#');
    CHECK(grid.GetCellTypeSymbol(floor_id) == '.');
    CHECK(grid.GetCellTypeSymbol(door_id)  == 'D');
  }

  SECTION("GetCellTypeID returns 0 for unknown names") {
    grid.AddCellType("Wall", "Solid wall", '#');

    CHECK(grid.GetCellTypeID("Wall") == 1);
    CHECK(grid.GetCellTypeID("NotARealType") == 0); // default/fallback
  }
}

TEST_CASE("Indexing and mutation via operator[] with (x,y) pair", "[core]") {
  using position = std::pair<size_t,size_t>;
  cse498::WorldGrid grid = MakeSmallGrid3x2();

  size_t wall_id  = grid.GetCellTypeID("Wall");
  size_t floor_id = grid.GetCellTypeID("Floor");
  size_t door_id  = grid.GetCellTypeID("Door");

  REQUIRE(wall_id  != 0);
  REQUIRE(floor_id != 0);
  REQUIRE(door_id  != 0);

  SECTION("Setting and getting values using (x,y) pair") {
    // Set up a simple pattern:
    // row 0: Wall Floor Floor
    // row 1: Floor Door Wall
    grid[position{0, 0}] = wall_id;
    grid[position{1, 0}] = floor_id;
    grid[position{2, 0}] = floor_id;

    grid[position{0, 1}] = floor_id;
    grid[position{1, 1}] = door_id;
    grid[position{2, 1}] = wall_id;

    CHECK(grid[position{0, 0}] == wall_id);
    CHECK(grid[position{1, 0}] == floor_id);
    CHECK(grid[position{2, 0}] == floor_id);

    CHECK(grid[position{0, 1}] == floor_id);
    CHECK(grid[position{1, 1}] == door_id);
    CHECK(grid[position{2, 1}] == wall_id);
  }
}

TEST_CASE("Indexing and symbol lookup via WorldPosition", "[core]") {
  auto grid = MakeSmallGrid3x2();

  size_t wall_id  = grid.GetCellTypeID("Wall");
  size_t floor_id = grid.GetCellTypeID("Floor");

  REQUIRE(wall_id  != 0);
  REQUIRE(floor_id != 0);

  // Construct positions exactly on integer coordinates.
  // Assumes WorldPosition(double x, double y) exists and
  // CellX()/CellY() map (1.0, 0.0) -> (1, 0), etc.
  cse498::WorldPosition pos_wall(0.0, 0.0);
  cse498::WorldPosition pos_floor(1.0, 0.0);

  grid[pos_wall]  = wall_id;
  grid[pos_floor] = floor_id;


  SECTION("IsValid(WorldPosition) respects grid bounds") {
    CHECK(grid.IsValid(pos_wall));
    CHECK(grid.IsValid(pos_floor));

    cse498::WorldPosition outside(-1.0, 0.0);
    CHECK_FALSE(grid.IsValid(outside));
  }

  SECTION("operator[](WorldPosition) reads underlying cell IDs") {
    CHECK(grid[pos_wall]  == wall_id);
    CHECK(grid[pos_floor] == floor_id);
  }

  SECTION("GetSymbol(WorldPosition) returns correct symbol") {
    CHECK(grid.GetSymbol(pos_wall)  == '#'); // Wall
    CHECK(grid.GetSymbol(pos_floor) == '.'); // Floor
  }
}

TEST_CASE("Resize preserves overlapping region and fills new cells with default_type", "[core]") {
  using position = std::pair<size_t,size_t>;
  auto grid = MakeSmallGrid3x2();

  size_t wall_id  = grid.GetCellTypeID("Wall");
  size_t floor_id = grid.GetCellTypeID("Floor");

  REQUIRE(wall_id  != 0);
  REQUIRE(floor_id != 0);

  // Original 3x2:
  // [0,0] = Wall, [1,0] = Floor, [2,0] = Wall
  // [0,1] = Floor, [1,1] = Wall, [2,1] = Floor
  grid[position{0, 0}] = wall_id;
  grid[position{1, 0}] = floor_id;
  grid[position{2, 0}] = wall_id;

  grid[position{0, 1}] = floor_id;
  grid[position{1, 1}] = wall_id;
  grid[position{2, 1}] = floor_id;

  SECTION("Resize to larger grid keeps existing cells and sets new ones to default_type") {
    // Resize to 4x3; default_type = 0 (Unknown)
    grid.Resize(4, 3, 0);

    CHECK(grid.GetWidth()  == 4);
    CHECK(grid.GetHeight() == 3);
    CHECK(grid.GetNumCells() == 12);

    // Overlapping region preserved
    CHECK(grid[position{0, 0}] == wall_id);
    CHECK(grid[position{1, 0}] == floor_id);
    CHECK(grid[position{2, 0}] == wall_id);

    CHECK(grid[position{0, 1}] == floor_id);
    CHECK(grid[position{1, 1}] == wall_id);
    CHECK(grid[position{2, 1}] == floor_id);

    // New cells (rightmost column and bottom row) should be default_type (0)
    CHECK(grid[position{3, 0}] == 0);
    CHECK(grid[position{3, 1}] == 0);
    CHECK(grid[position{0, 2}] == 0);
    CHECK(grid[position{3, 2}] == 0);
  }
}
*/
