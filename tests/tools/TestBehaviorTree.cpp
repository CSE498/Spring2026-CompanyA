/**
 * @file TestBehaviorTree.cpp
 * @author Jacob Bettinger
 *
 * Catch2 unit tests for BehaviorTree.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/BehaviorTree.hpp"

#include <vector>
#include <string>

using namespace cse498::BehaviorTrees;

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

TEST_CASE("Blackboard: Overwrite existing key", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<int>("value", 1);
    bb.Set<int>("value", 99);

    REQUIRE(bb.Get<int>("value") == 99);
}

TEST_CASE("Blackboard: Has returns false for missing key", "[Blackboard]")
{
    Blackboard bb;
    REQUIRE_FALSE(bb.Has("nonexistent"));
}

TEST_CASE("Blackboard: GetKeys returns empty on empty board", "[Blackboard]")
{
    Blackboard bb;
    REQUIRE(bb.GetKeys().size() == 0);
}

TEST_CASE("Blackboard: Remove nonexistent key does not throw", "[Blackboard]")
{
    Blackboard bb;
    REQUIRE_NOTHROW(bb.Remove("nonexistent"));
}

TEST_CASE("Blackboard: Clear on empty board does not throw", "[Blackboard]")
{
    Blackboard bb;
    REQUIRE_NOTHROW(bb.Clear());
}

TEST_CASE("Blackboard: Supports float type", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<float>("speed", 3.14f);
    REQUIRE(bb.Get<float>("speed") == Approx(3.14f));
}

TEST_CASE("Blackboard: Supports bool type", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<bool>("alive", true);
    REQUIRE(bb.Get<bool>("alive"));
    bb.Set<bool>("alive", false);
    REQUIRE(!bb.Get<bool>("alive"));
}

TEST_CASE("Blackboard: Type mismatch returns default", "[Blackboard]")
{
    Blackboard bb;
    bb.Set<int>("key", 42);
    REQUIRE(bb.Get<std::string>("key", "fallback") == "fallback");
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
        int value = ctx.mBlackboard.Get<int>("counter", 0);
        ctx.mBlackboard.Set<int>("counter", value + 1);
        return Node::Status::Success;
    });

    action.Tick(ctx);
    REQUIRE(bb.Get<int>("counter") == 1);
    action.Tick(ctx);
    REQUIRE(bb.Get<int>("counter") == 2);
}

TEST_CASE("Action: GetName returns correct name", "[Action]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Action action("MyAction", [](ExecutionContext&) { return Node::Status::Success; });
    REQUIRE(action.GetName() == "MyAction");
}

TEST_CASE("Action: Reset does not throw", "[Action]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Action action("Resettable", [](ExecutionContext&) { return Node::Status::Success; });
    action.Tick(ctx);
    REQUIRE_NOTHROW(action.Reset());
    REQUIRE(action.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Action: Conditional status based on blackboard", "[Action]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);
    bb.Set<bool>("flag", false);

    Action action("Conditional", [](ExecutionContext& ctx) {
        return ctx.mBlackboard.Get<bool>("flag", false)
            ? Node::Status::Success
            : Node::Status::Failure;
    });

    REQUIRE(action.Tick(ctx) == Node::Status::Failure);
    bb.Set<bool>("flag", true);
    REQUIRE(action.Tick(ctx) == Node::Status::Success);
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

TEST_CASE("Selector: Short-circuits on first success", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("ShortCircuitSelector");
    sel.AddChild(std::make_unique<Action>("Success",
        [](ExecutionContext& ctx) {
            ctx.mBlackboard.Set<int>("visited", ctx.mBlackboard.Get<int>("visited", 0) + 1);
            return Node::Status::Success;
        }));
    sel.AddChild(std::make_unique<Action>("ShouldNotRun",
        [](ExecutionContext& ctx) {
            ctx.mBlackboard.Set<int>("visited", ctx.mBlackboard.Get<int>("visited", 0) + 10);
            return Node::Status::Success;
        }));

    sel.Tick(ctx);
    REQUIRE(bb.Get<int>("visited") == 1);
}

TEST_CASE("Selector: GetName returns correct name", "[Selector]")
{
    Selector sel("MySel");
    REQUIRE(sel.GetName() == "MySel");
}

TEST_CASE("Selector: Resumes from beginning after Reset", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);
    int callCount = 0;

    Selector sel("ResetSelector");
    sel.AddChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));
    sel.AddChild(std::make_unique<Action>("Running",
        [&callCount](ExecutionContext&) {
            callCount++;
            return Node::Status::Running;
        }));

    sel.Tick(ctx);
    REQUIRE(callCount == 1);
    sel.Reset();
    sel.Tick(ctx);
    REQUIRE(callCount == 2);
}

TEST_CASE("Selector: Single successful child returns Success", "[Selector]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Selector sel("SingleSuccess");
    sel.AddChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(sel.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Selector: GetChildren returns correct count", "[Selector]")
{
    Selector sel("ChildCountSelector");
    sel.AddChild(std::make_unique<Action>("A",
        [](ExecutionContext&) { return Node::Status::Success; }));
    sel.AddChild(std::make_unique<Action>("B",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(sel.GetChildren().size() == 2);
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

TEST_CASE("Sequence: Returns Running when child is Running", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Sequence seq("RunningSequence");
    seq.AddChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));
    seq.AddChild(std::make_unique<Action>("Running",
        [](ExecutionContext&) { return Node::Status::Running; }));

    REQUIRE(seq.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("Sequence: Short-circuits on first failure", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Sequence seq("ShortCircuitSequence");
    seq.AddChild(std::make_unique<Action>("Fail",
        [](ExecutionContext& ctx) {
            ctx.mBlackboard.Set<int>("visited", ctx.mBlackboard.Get<int>("visited", 0) + 1);
            return Node::Status::Failure;
        }));
    seq.AddChild(std::make_unique<Action>("ShouldNotRun",
        [](ExecutionContext& ctx) {
            ctx.mBlackboard.Set<int>("visited", ctx.mBlackboard.Get<int>("visited", 0) + 10);
            return Node::Status::Success;
        }));

    seq.Tick(ctx);
    REQUIRE(bb.Get<int>("visited") == 1);
}

TEST_CASE("Sequence: GetName returns correct name", "[Sequence]")
{
    Sequence seq("MySeq");
    REQUIRE(seq.GetName() == "MySeq");
}

TEST_CASE("Sequence: Resumes from beginning after Reset", "[Sequence]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);
    int callCount = 0;

    Sequence seq("ResetSequence");
    seq.AddChild(std::make_unique<Action>("Counter",
        [&callCount](ExecutionContext&) {
            callCount++;
            return Node::Status::Success;
        }));
    seq.AddChild(std::make_unique<Action>("Running",
        [](ExecutionContext&) { return Node::Status::Running; }));

    seq.Tick(ctx);
    REQUIRE(callCount == 1);
    seq.Reset();
    seq.Tick(ctx);
    REQUIRE(callCount == 2);
}

TEST_CASE("Sequence: GetChildren returns correct count", "[Sequence]")
{
    Sequence seq("ChildCountSequence");
    seq.AddChild(std::make_unique<Action>("A",
        [](ExecutionContext&) { return Node::Status::Success; }));
    seq.AddChild(std::make_unique<Action>("B",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(seq.GetChildren().size() == 2);
}

// =============================================================================
// INVERT TESTS
// =============================================================================

TEST_CASE("Invert: Inverts Success to Failure", "[Invert]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Invert inv("TestInvert");
    inv.SetChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(inv.Tick(ctx) == Node::Status::Failure);
}

TEST_CASE("Invert: Inverts Failure to Success", "[Invert]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Invert inv("TestInvert");
    inv.SetChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));

    REQUIRE(inv.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Invert: Passes through Running unchanged", "[Invert]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Invert inv("TestInvert");
    inv.SetChild(std::make_unique<Action>("Running",
        [](ExecutionContext&) { return Node::Status::Running; }));

    REQUIRE(inv.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("Invert: No child returns Failure", "[Invert]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Invert inv("EmptyInvert");
    REQUIRE(inv.Tick(ctx) == Node::Status::Failure);
}

TEST_CASE("Invert: GetName returns correct name", "[Invert]")
{
    Invert inv("MyInvert");
    REQUIRE(inv.GetName() == "MyInvert");
}

TEST_CASE("Invert: Consistent across multiple ticks", "[Invert]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    Invert inv("ConsistentInvert");
    inv.SetChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));

    REQUIRE(inv.Tick(ctx) == Node::Status::Success);
    REQUIRE(inv.Tick(ctx) == Node::Status::Success);
    REQUIRE(inv.Tick(ctx) == Node::Status::Success);
}

TEST_CASE("Invert: GetChild returns set child", "[Invert]")
{
    Invert inv("ChildInvert");
    inv.SetChild(std::make_unique<Action>("Child",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(inv.GetChild() != nullptr);
    REQUIRE(inv.GetChild()->GetName() == "Child");
}

// =============================================================================
// CONTINUALLY REPEAT TESTS
// =============================================================================

TEST_CASE("ContinuallyRepeat: Always returns Running on Success child", "[ContinuallyRepeat]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    ContinuallyRepeat repeat("TestRepeat");
    repeat.SetChild(std::make_unique<Action>("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("ContinuallyRepeat: Always returns Running on Failure child", "[ContinuallyRepeat]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    ContinuallyRepeat repeat("TestRepeat");
    repeat.SetChild(std::make_unique<Action>("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));

    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("ContinuallyRepeat: No child returns Running", "[ContinuallyRepeat]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    ContinuallyRepeat repeat("EmptyRepeat");
    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("ContinuallyRepeat: Child action is called repeatedly", "[ContinuallyRepeat]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    ContinuallyRepeat repeat("CountingRepeat");
    repeat.SetChild(std::make_unique<Action>("Counter", [](ExecutionContext& ctx) {
        int value = ctx.mBlackboard.Get<int>("counter", 0);
        ctx.mBlackboard.Set<int>("counter", value + 1);
        return Node::Status::Success;
    }));

    repeat.Tick(ctx);
    repeat.Tick(ctx);
    repeat.Tick(ctx);

    REQUIRE(bb.Get<int>("counter") == 3);
}

TEST_CASE("ContinuallyRepeat: GetName returns correct name", "[ContinuallyRepeat]")
{
    ContinuallyRepeat repeat("MyRepeat");
    REQUIRE(repeat.GetName() == "MyRepeat");
}

TEST_CASE("ContinuallyRepeat: Still Running when child is Running", "[ContinuallyRepeat]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    ContinuallyRepeat repeat("RunningRepeat");
    repeat.SetChild(std::make_unique<Action>("Running",
        [](ExecutionContext&) { return Node::Status::Running; }));

    REQUIRE(repeat.Tick(ctx) == Node::Status::Running);
}

TEST_CASE("ContinuallyRepeat: GetChild returns set child", "[ContinuallyRepeat]")
{
    ContinuallyRepeat repeat("ChildRepeat");
    repeat.SetChild(std::make_unique<Action>("Child",
        [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(repeat.GetChild() != nullptr);
    REQUIRE(repeat.GetChild()->GetName() == "Child");
}

// =============================================================================
// TREE BUILDER TESTS
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

TEST_CASE("TreeBuilder: Creates Invert correctly", "[TreeBuilder]")
{
    auto inv = TreeBuilder::Inv("TestInvert");
    REQUIRE(inv != nullptr);
    REQUIRE(inv->GetName() == "TestInvert");
}

TEST_CASE("TreeBuilder: Creates ContinuallyRepeat correctly", "[TreeBuilder]")
{
    auto repeat = TreeBuilder::Repeat("TestRepeat");
    REQUIRE(repeat != nullptr);
    REQUIRE(repeat->GetName() == "TestRepeat");
}

TEST_CASE("TreeBuilder: Act node executes correctly", "[TreeBuilder]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    auto act = TreeBuilder::Act("ActNode",
        [](ExecutionContext&) { return Node::Status::Success; });

    REQUIRE(act->Tick(ctx) == Node::Status::Success);
}

TEST_CASE("TreeBuilder: Seq node with children executes correctly", "[TreeBuilder]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    auto seq = TreeBuilder::Seq("BuiltSeq");
    seq->AddChild(TreeBuilder::Act("A", [](ExecutionContext&) { return Node::Status::Success; }));
    seq->AddChild(TreeBuilder::Act("B", [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(seq->Tick(ctx) == Node::Status::Success);
}

TEST_CASE("TreeBuilder: Sel node with children executes correctly", "[TreeBuilder]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    auto sel = TreeBuilder::Sel("BuiltSel");
    sel->AddChild(TreeBuilder::Act("Fail", [](ExecutionContext&) { return Node::Status::Failure; }));
    sel->AddChild(TreeBuilder::Act("Success", [](ExecutionContext&) { return Node::Status::Success; }));

    REQUIRE(sel->Tick(ctx) == Node::Status::Success);
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

TEST_CASE("Integration: Selector with Sequences as children", "[Integration]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);
    bb.Set<bool>("hasAmmo", false);
    bb.Set<bool>("canMelee", true);

    auto root = TreeBuilder::Sel("Root");

    auto shootSeq = TreeBuilder::Seq("ShootSequence");
    shootSeq->AddChild(TreeBuilder::Act("CheckAmmo", [](ExecutionContext& ctx) {
        return ctx.mBlackboard.Get<bool>("hasAmmo", false)
            ? Node::Status::Success : Node::Status::Failure;
    }));
    shootSeq->AddChild(TreeBuilder::Act("Shoot", [](ExecutionContext& ctx) {
        ctx.mBlackboard.Set<bool>("shot", true);
        return Node::Status::Success;
    }));

    auto meleeSeq = TreeBuilder::Seq("MeleeSequence");
    meleeSeq->AddChild(TreeBuilder::Act("CheckMelee", [](ExecutionContext& ctx) {
        return ctx.mBlackboard.Get<bool>("canMelee", false)
            ? Node::Status::Success : Node::Status::Failure;
    }));
    meleeSeq->AddChild(TreeBuilder::Act("Melee", [](ExecutionContext& ctx) {
        ctx.mBlackboard.Set<bool>("meleed", true);
        return Node::Status::Success;
    }));

    root->AddChild(std::move(shootSeq));
    root->AddChild(std::move(meleeSeq));

    REQUIRE(root->Tick(ctx) == Node::Status::Success);
    REQUIRE(!bb.Get<bool>("shot", false));
    REQUIRE(bb.Get<bool>("meleed", false));
}

TEST_CASE("Integration: Invert used as condition guard in Sequence", "[Integration]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);
    bb.Set<bool>("isDead", false);

    auto seq = TreeBuilder::Seq("GuardedSequence");

    auto inv = TreeBuilder::Inv("NotDead");
    inv->SetChild(TreeBuilder::Act("IsDead", [](ExecutionContext& ctx) {
        return ctx.mBlackboard.Get<bool>("isDead", false)
            ? Node::Status::Success : Node::Status::Failure;
    }));

    seq->AddChild(std::move(inv));
    seq->AddChild(TreeBuilder::Act("Act", [](ExecutionContext& ctx) {
        ctx.mBlackboard.Set<bool>("acted", true);
        return Node::Status::Success;
    }));

    REQUIRE(seq->Tick(ctx) == Node::Status::Success);
    REQUIRE(bb.Get<bool>("acted", false));

    bb.Set<bool>("isDead", true);
    bb.Set<bool>("acted", false);

    REQUIRE(seq->Tick(ctx) == Node::Status::Failure);
    REQUIRE(!bb.Get<bool>("acted", false));
}

TEST_CASE("Integration: ContinuallyRepeat wrapping a Sequence", "[Integration]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    auto repeat = TreeBuilder::Repeat("RepeatLoop");

    auto seq = TreeBuilder::Seq("InnerSeq");
    seq->AddChild(TreeBuilder::Act("Increment", [](ExecutionContext& ctx) {
        int v = ctx.mBlackboard.Get<int>("ticks", 0);
        ctx.mBlackboard.Set<int>("ticks", v + 1);
        return Node::Status::Success;
    }));

    repeat->SetChild(std::move(seq));

    repeat->Tick(ctx);
    repeat->Tick(ctx);
    repeat->Tick(ctx);

    REQUIRE(bb.Get<int>("ticks") == 3);
    REQUIRE(repeat->Tick(ctx) == Node::Status::Running);
}

TEST_CASE("Integration: Deep nested tree executes correctly", "[Integration]")
{
    Blackboard bb;
    ExecutionContext ctx(bb);

    auto root = TreeBuilder::Sel("Root");

    auto outerSeq = TreeBuilder::Seq("OuterSeq");

    auto innerSel = TreeBuilder::Sel("InnerSel");
    innerSel->AddChild(TreeBuilder::Act("Fail",
        [](ExecutionContext&) { return Node::Status::Failure; }));
    innerSel->AddChild(TreeBuilder::Act("Success",
        [](ExecutionContext&) { return Node::Status::Success; }));

    outerSeq->AddChild(std::move(innerSel));
    outerSeq->AddChild(TreeBuilder::Act("FinalAct", [](ExecutionContext& ctx) {
        ctx.mBlackboard.Set<bool>("done", true);
        return Node::Status::Success;
    }));

    root->AddChild(std::move(outerSeq));

    REQUIRE(root->Tick(ctx) == Node::Status::Success);
    REQUIRE(bb.Get<bool>("done", false));
}