#include "BehaviorTree.hpp"

namespace cse498 {
namespace BehaviorTrees {

// ============================================================
// BLACKBOARD IMPLEMENTATION
// ============================================================

bool Blackboard::Has(const std::string& key) const { return mData.find(key) != mData.end(); }

void Blackboard::Remove(const std::string& key) { mData.erase(key); }

void Blackboard::Clear() { mData.clear(); }

std::vector<std::string> Blackboard::GetKeys() const {
    std::vector<std::string> keys;
    keys.reserve(mData.size());
    for (const auto& pair: mData) {
        keys.push_back(pair.first);
    }
    return keys;
}

// ============================================================
// EXECUTION CONTEXT
// ============================================================

ExecutionContext::ExecutionContext(Blackboard& bb) : mBlackboard(bb) {}

// ============================================================
// NODE BASE IMPLEMENTATION
// ============================================================

Node::Status Node::Tick(ExecutionContext& context) {
    OnEnter(context);
    Status status = OnUpdate(context);
    OnExit(context, status);
    return status;
}

void Node::OnEnter(ExecutionContext&) {
    // Default: no-op
}

void Node::OnExit(ExecutionContext&, Status) {
    // Default: no-op
}

// ============================================================
// COMPOSITE
// ============================================================

void Composite::AddChild(std::unique_ptr<Node> child) { mChildren.push_back(std::move(child)); }

void Composite::Reset() {
    mCurrentChild = 0;
    for (auto& ch: mChildren) {
        ch->Reset();
    }
}

const std::vector<std::unique_ptr<Node>>& Composite::GetChildren() const { return mChildren; }

// ============================================================
// DECORATOR
// ============================================================

void Decorator::SetChild(std::unique_ptr<Node> childNode) { mChild = std::move(childNode); }

void Decorator::Reset() {
    if (mChild) {
        mChild->Reset();
    }
}

const std::unique_ptr<Node>& Decorator::GetChild() const { return mChild; }

// ============================================================
// ACTION
// ============================================================

Action::Action(const std::string& name, ActionFunc action) : mName(name), mAction(std::move(action)) {}

std::string Action::GetName() const { return mName; }

void Action::Reset() {
    // Typically stateless
}

Node::Status Action::OnUpdate(ExecutionContext& context) { return mAction(context); }

// ============================================================
// SELECTOR
// ============================================================

Selector::Selector(const std::string& name) : mName(name) {}

std::string Selector::GetName() const { return mName; }

Node::Status Selector::OnUpdate(ExecutionContext& context) {
    while (mCurrentChild < mChildren.size()) {
        Status status = mChildren[mCurrentChild]->Tick(context);

        if (status == Status::Success) {
            mCurrentChild = 0;
            return Status::Success;
        }

        if (status == Status::Running)
            return Status::Running;

        mCurrentChild++;
    }

    mCurrentChild = 0;
    return Status::Failure;
}

// ============================================================
// SEQUENCE
// ============================================================

Sequence::Sequence(const std::string& name) : mName(name) {}

std::string Sequence::GetName() const { return mName; }

Node::Status Sequence::OnUpdate(ExecutionContext& context) {
    while (mCurrentChild < mChildren.size()) {
        Status status = mChildren[mCurrentChild]->Tick(context);

        if (status == Status::Failure) {
            mCurrentChild = 0;
            return Status::Failure;
        }

        if (status == Status::Running)
            return Status::Running;

        mCurrentChild++;
    }

    mCurrentChild = 0;
    return Status::Success;
}

// ============================================================
// INVERT
// ============================================================

Invert::Invert(const std::string& name) : mName(name) {}

std::string Invert::GetName() const { return mName; }

Node::Status Invert::OnUpdate(ExecutionContext& context) {
    if (!mChild)
        return Status::Failure;

    Status status = mChild->Tick(context);

    if (status == Status::Success)
        return Status::Failure;

    if (status == Status::Failure)
        return Status::Success;

    return Status::Running;
}

// ============================================================
// CONTINUALLY REPEAT
// ============================================================

ContinuallyRepeat::ContinuallyRepeat(const std::string& name) : mName(name) {}

std::string ContinuallyRepeat::GetName() const { return mName; }

Node::Status ContinuallyRepeat::OnUpdate(ExecutionContext& context) {
    if (!mChild)
        return Status::Running;

    Status status = mChild->Tick(context);

    if (status == Status::Success || status == Status::Failure) {
        mChild->Reset();
    }

    return Status::Running;
}

// ============================================================
// TREE BUILDER
// ============================================================

std::unique_ptr<Sequence> TreeBuilder::Seq(const std::string& name) { return std::make_unique<Sequence>(name); }

std::unique_ptr<Selector> TreeBuilder::Sel(const std::string& name) { return std::make_unique<Selector>(name); }

std::unique_ptr<Action> TreeBuilder::Act(const std::string& name, Action::ActionFunc func) {
    return std::make_unique<Action>(name, std::move(func));
}

std::unique_ptr<Invert> TreeBuilder::Inv(const std::string& name) { return std::make_unique<Invert>(name); }

std::unique_ptr<ContinuallyRepeat> TreeBuilder::Repeat(const std::string& name) {
    return std::make_unique<ContinuallyRepeat>(name);
}

} // namespace BehaviorTrees
} // namespace cse498
