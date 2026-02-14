/**
 * @file TestBehaviorTree.cpp
 * @author Jacob Bettinger
 *
 * Catch2 unit tests for BehaviorTree.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"


#include "../../source/tools/BehaviorTree/BehaviorTree.hpp"

#include <vector>
#include <string>

using namespace Pathfinding::BehaviorTrees;

// =============================================================================
// BLACKBOARD TESTS
// =============================================================================

TEST_CASE("Blackboard: Set and Get basic types", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<int>("health", 100);
    bb.Set<std::string>("name", "Enemy");

    REQUIRE(bb.Get<int>("health") == 100);
    REQUIRE(bb.Get<std::string>("name") == "Enemy");
}

TEST_CASE("Blackboard: Get with default value for missing key", "[Blackboard]")
{
    Blackboard bb;
    REQUIRE(bb.Get<int>("missing", 42) == 42);
    REQUIRE(bb.Get<std::string>("missing", "default") == "default");
}

TEST_CASE("Blackboard: Has, Remove, and Clear", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<int>("a", 1);
    bb.Set<int>("b", 2);

    REQUIRE(bb.Has("a"));
    bb.Remove("a");
    REQUIRE_FALSE(bb.Has("a"));

    bb.Clear();
    REQUIRE_FALSE(bb.Has("b"));
}

TEST_CASE("Blackboard: GetKeys returns all keys", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<int>("alpha", 1);
    bb.Set<int>("beta", 2);

    auto keys = bb.GetKeys();
    REQUIRE(keys.size() == 2);
}

// =============================================================================
// ACTION TESTS
// =============================================================================

TEST_CASE("Action: Returns correct status", "[Action]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Action successAction("Success", [](ExecutionContext&) { return Node::Status::Success; });
    Action failAction("Fail", [](ExecutionContext&) { return Node::Status::Failure; });
    Action runningAction("Running", [](ExecutionContext&) { return Node::Status::Running; });

    REQUIRE(successAction.Tick(ctx) == Node::Status::Success);
    REQUIRE(failAction.Tick(ctx) == Node::Status::Failure);
    REQUIRE(runningAction.Tick(ctx) == Node::Status::Running);
    REQUIRE(successAction.GetName() == "Success");
}

TEST_CASE("Action: Can access and modify blackboard", "[Action]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Action action("Counter", [](ExecutionContext& ctx) {
        int value = ctx.blackboard.Get<int>("counter", 0);
        ctx.blackboard.Set<int>("counter", value + 1);
        return Node::Status::Success;
    });

    action.Tick(ctx);
    REQUIRE(bb.Get<int>("counter") == 1);
    action.Tick(ctx);
    REQUIRE(bb.Get<int>("counter") == 2);
}

// =============================================================================
// SELECTOR TESTS
// =============================================================================

TEST_CASE("Selector: Empty selector returns Failure", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("EmptySelector");
    REQUIRE(sel.Tick(ctx) == Node::Status::Failure);
}

TEST_CASE("Selector: Returns Success on first successful child", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("TestSelector");
    sel.AddChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));
    sel.AddChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(sel.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Selector: Returns Failure when all children fail", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("FailSelector");
    sel.AddChild(std::make_unique<Action>("Fail1",
        [](ExecutionContext&) { return Node::Status::Failure; }));
    sel.AddChild(std::make_unique<Action>("Fail2",
        [](ExecutionContext&) { return Node::Status::Failure; }));

    REQUIRE(sel.Tick(ctx) == Node::Status::Failure);
}

TEST_CASE("Selector: Returns Running when child is running", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("RunningSelector");
    sel.AddChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));
    sel.AddChild(std::make_unique<Action>("Running",
        [](ExecutionContext&) { return Node::Status::Running; }));

    REQUIRE(sel.Tick(ctx) == Node::Status::Running);
}

// =============================================================================
// SEQUENCE TESTS
// =============================================================================

TEST_CASE("Sequence: Empty sequence returns Success", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Sequence seq("EmptySequence");
    REQUIRE(seq.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Sequence: Returns Success when all children succeed", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Sequence seq("SuccessSequence");
    seq.AddChild(std::make_unique<Action>("Success1",
        [](ExecutionContext&) { return Node::Status::Success; }));
    seq.AddChild(std::make_unique<Action>("Success2",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(seq.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Sequence: Returns Failure on first failing child", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Sequence seq("FailSequence");
    seq.AddChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));
    seq.AddChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));

    REQUIRE(seq.Tick(ctx) == Node::Status::Failure);
}

// =============================================================================
// TREE BUILDER TEST
// =============================================================================

TEST_CASE("TreeBuilder: Creates nodes correctly", "[TreeBuilder]")
{
    auto seq = TreeBuilder::Seq("TestSequence");
    auto sel = TreeBuilder::Sel("TestSelector");
    auto act = TreeBuilder::Act("TestAction",
        [](ExecutionContext&) { return Node::Status::Success; });

    REQUIRE(seq != nullptr);
    REQUIRE(seq->GetName() == "TestSequence");
    REQUIRE(sel != nullptr);
    REQUIRE(act != nullptr);
}
