/**
 * @file TestActionMap.cpp
 * @author Ty Maksimowski
 *
 * Catch2 unit tests for ActionMap.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/ActionMap.hpp"

#include <algorithm>
#include <string>

using namespace cse498;


TEST_CASE("AddFunction and Trigger for void()", "[ActionMap]")
{
    ActionMap map;

    bool called = false;
    REQUIRE(map.AddFunction("Jump", [&](){ called = true; }, "Makes the player jump"));

    REQUIRE(map.Size() == 1);
    REQUIRE(map.Has("Jump"));

    REQUIRE(map.GetDescription("Jump").has_value());
    REQUIRE(*map.GetDescription("Jump") == "Makes the player jump");

    REQUIRE(map.Trigger("Jump"));
    REQUIRE(called);
}

TEST_CASE("SetDescription works and fails predictably", "[ActionMap]")
{
    ActionMap map;
    REQUIRE(map.AddFunction("Run", []{}));

    REQUIRE(map.SetDescription("Run", "Makes the player run"));
    REQUIRE(map.GetDescription("Run").has_value());
    REQUIRE(*map.GetDescription("Run") == "Makes the player run");

    REQUIRE(map.SetDescription("Run", std::nullopt)); // clear description
    REQUIRE_FALSE(map.GetDescription("Run").has_value());

    REQUIRE_FALSE(map.SetDescription("Missing", "No action")); // action doesn't exist
}

TEST_CASE("AddFunction rejects duplicates and invalid names", "[ActionMap]")
{
    ActionMap map;

    REQUIRE(map.AddFunction("A", []{}));
    REQUIRE_FALSE(map.AddFunction("A", []{})); // duplicate
    REQUIRE_FALSE(map.AddFunction("", []{})); // invalid name
}

TEST_CASE("RemoveFunction works and fails predictably", "[ActionMap]")
{
    ActionMap map;
    REQUIRE(map.AddFunction("OpenChest", []{}));

    REQUIRE(map.RemoveFunction("OpenChest"));
    REQUIRE_FALSE(map.RemoveFunction("OpenChest")); // already removed
    REQUIRE_FALSE(map.RemoveFunction("")); // invalid name
}

TEST_CASE("Rename works and avoids overwriting", "[ActionMap]")
{
    ActionMap map;
    REQUIRE(map.AddFunction("Old", []{}));
    REQUIRE(map.AddFunction("Existing", []{}));

    REQUIRE(map.Rename("Old", "New"));
    REQUIRE_FALSE(map.Has("Old"));
    REQUIRE(map.Has("New"));

    REQUIRE_FALSE(map.Rename("New", "Existing")); // would overwrite
    REQUIRE_FALSE(map.Rename("Missing", "X")); // old missing
}

TEST_CASE("ListActions returns all names (order not guaranteed)", "[ActionMap]")
{
    ActionMap map;
    REQUIRE(map.AddFunction("A", []{}));
    REQUIRE(map.AddFunction("B", []{}));
    REQUIRE(map.AddFunction("C", []{}));

    auto list = map.ListActions();
    REQUIRE(list.size() == 3);

    REQUIRE(std::find(list.begin(), list.end(), "A") != list.end());
    REQUIRE(std::find(list.begin(), list.end(), "B") != list.end());
    REQUIRE(std::find(list.begin(), list.end(), "C") != list.end());
}

TEST_CASE("Typed actions: arguments + return values", "[ActionMap]")
{
    ActionMap map;

    // Add typed function (int,int)->int
    REQUIRE(map.AddFunction<int, int, int>(
                "Add",
                std::function<int(int,int)>([](int a, int b){ return a + b; }),
                "Adds two ints"
            ));

    auto out = map.Trigger<int, int, int>("Add", 2, 5);
    REQUIRE(out.has_value());
    REQUIRE(*out == 7);

    // Add typed void(int) action
    int sink = 0;
    REQUIRE(map.AddFunction<void, int>(
                "SetSink",
                std::function<void(int)>([&](int v){ sink = v; })
            ));

    REQUIRE(map.Trigger<void>("SetSink", 99));
    REQUIRE(sink == 99);
}

TEST_CASE("Typed trigger fails cleanly on signature mismatch", "[ActionMap]")
{
    ActionMap map;

    REQUIRE(map.AddFunction<int, int, int>(
                "Add",
                std::function<int(int,int)>([](int a, int b){ return a + b; })
            ));

    // Wrong signature: trying to trigger as void(int,int)
    REQUIRE_FALSE(map.Trigger<void, int, int>("Add", 1, 2));

    // Wrong signature: trying to trigger with different arg types
    auto out = map.Trigger<int, std::string, std::string>("Add", "1", "2");
    REQUIRE_FALSE(out.has_value());
}

TEST_CASE("TriggerVoid fails when action doesn't exist", "[ActionMap]")
{
    ActionMap map;
    REQUIRE_FALSE(map.TriggerVoid<>("MissingAction"));
}

TEST_CASE("TriggerTyped fails when action doesn't exist", "[ActionMap]")
{
    ActionMap map;
    auto out = map.Trigger<int, int, int>("MissingAction", 1, 2);
    REQUIRE_FALSE(out.has_value());
}
