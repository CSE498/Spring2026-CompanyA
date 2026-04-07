#include "../../source/tools/InteractiveWorld/DataMap.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

TEST_CASE("Test DataMap Set and Get Functions") {
  cse498::DataMap map;

  CHECK(map.Size() == 0);
  map.Set("Health", 15);
  CHECK(map.Get<int>("Health") == 15);
  CHECK(map.Size() == 1);

  map.Set("Health", 30);
  CHECK(map.Get<int>("Health") == 30);
  CHECK(map.Size() == 1);

  map.Set("Health", 0);
  CHECK(map.Get<int>("Health") == 0);
  CHECK(map.Size() == 1);

  map.Set("Speed", 15.0);
  CHECK(map.Get<double>("Speed") == 15.0);
  CHECK(map.Size() == 2);

  map.Set("Stamina", 50.0);
  CHECK(map.Get<double>("Stamina") == 50.0);
  CHECK(map.Size() == 3);
}

TEST_CASE("Test DataMap Has Function") {
  cse498::DataMap map;

  CHECK(map.Has("Health") == false);
  CHECK(map.Size() == 0);

  map.Set("Health", 15);
  CHECK(map.Has("Health") == true);
  CHECK(map.Size() == 1);

  map.Clear();
  CHECK(map.Has("Health") == false);
  CHECK(map.Size() == 0);

  map.Set("Health", 15);
  CHECK(map.Has("Health") == true);
  CHECK(map.Size() == 1);

  map.Remove("Health");
  CHECK(map.Has("Health") == false);
  CHECK(map.Size() == 0);
}

// TEST_CASE("Test DataMap Remove Function")
// {
// 	cse498::DataMap map;

// 	CHECK(map.Size() == 0);

// 	map.Set("Health", 15);
// 	CHECK(map.Has("Health") == true);
// 	CHECK(map.Size() == 1);

// 	map.Remove("Health");
// 	CHECK(map.Has("Health") == false);
// 	CHECK(map.Size() == 0);

// 	map.Set("Speed", 20.0);
// 	CHECK(map.Size() == 1);
// 	map.Set("Stamina", 50.0);
// 	CHECK(map.Size() == 2);
// 	map.Remove("Speed");
// 	CHECK(map.Size() == 1);
// 	map.Remove("Mana");
// 	CHECK(map.Size() == 1);
// 	map.Remove("Stamina");
// 	CHECK(map.Size() == 0);
// 	CHECK(map.Empty() == true);
// }

TEST_CASE("Test DataMap Clear and Empty Functions") {
  cse498::DataMap map;

  CHECK(map.Empty() == true);
  CHECK(map.Size() == 0);
  map.Clear();
  CHECK(map.Empty() == true);
  CHECK(map.Size() == 0);

  map.Set("Health", 15);
  CHECK(map.Empty() == false);
  map.Clear();
  CHECK(map.Empty() == true);
  CHECK(map.Size() == 0);

  map.Set("Health", 15);
  map.Set("Stamina", 25.0);
  map.Set("Mana", 35.0);
  CHECK(map.Size() == 3);
  CHECK(map.Empty() == false);

  map.Clear();
  CHECK(map.Empty() == true);
  CHECK(map.Size() == 0);
}