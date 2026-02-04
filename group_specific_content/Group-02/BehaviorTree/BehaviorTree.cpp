#include "BehaviorTree.h"
#include <algorithm>
#include <utility>

namespace Pathfinding {
namespace BehaviorTrees {


bool Blackboard::Has(const std::string& key) const {
    return data.find(key) != data.end();
}

void Blackboard::Remove(const std::string& key) {
    data.erase(key);
}

void Blackboard::Clear() {
    data.clear();
}

std::vector<std::string> Blackboard::GetKeys() const {
    std::vector<std::string> keys;
    keys.reserve(data.size());
    for (const auto& pair : data) {
        keys.push_back(pair.first);
    }
    return keys;
}

ExecutionContext::ExecutionContext(Blackboard& bb)
    : blackboard(bb) {}


Node::Status Node::Tick(ExecutionContext& context) {
    OnEnter(context);
    Status status = OnUpdate(context);
    OnExit(context, status);
    return status;
}

void Node::OnEnter(ExecutionContext& context) {
    // Default: do nothing
}

void Node::OnExit(ExecutionContext& context, Status status) {
    // Default: do nothing
}

// Composite implementation
void Composite::AddChild(std::unique_ptr<Node> child) {
    children.push_back(std::move(child));
}

void Composite::Reset() {
    currentChild = 0;
    for (auto& child : children) {
        child->Reset();
    }
}

const std::vector<std::unique_ptr<Node>>& Composite::GetChildren() const {
    return children;
}

// Strategy Method Implementation
Action::Action(const std::string& name, ActionFunc action)
    : name(name), action(action) {}

std::string Action::GetName() const {
    return name;
}

void Action::Reset() {
    // Actions typically don't have state to reset
}

Node::Status Action::OnUpdate(ExecutionContext& context) {
    return action(context);
}


// =============================================================================
// SELECTOR IMPLEMENTATION
// =============================================================================
Selector::Selector(const std::string& name) : name(name) {}

std::string Selector::GetName() const {
    return name;
}

Node::Status Selector::OnUpdate(ExecutionContext& context) {
    while (currentChild < children.size()) {
        Status status = children[currentChild]->Tick(context);
        
        switch (status) {
            case Status::Success:
                currentChild = 0;
                return Status::Success;
            case Status::Running:
                return Status::Running;
            case Status::Failure:
                currentChild++;
                continue;
        }
    }
    currentChild = 0;
    return Status::Failure;
}

// =============================================================================
// SEQUENCE IMPLEMENTATION
// =============================================================================
Sequence::Sequence(const std::string& name) : name(name) {}

std::string Sequence::GetName() const {
    return name;
}

Node::Status Sequence::OnUpdate(ExecutionContext& context) {
    while (currentChild < children.size()) {
        Status status = children[currentChild]->Tick(context);
        
        switch (status) {
            case Status::Failure:
                currentChild = 0;
                return Status::Failure;
            case Status::Running:
                return Status::Running;
            case Status::Success:
                currentChild++;
                if (currentChild >= children.size()) {
                    currentChild = 0;
                    return Status::Success;
                }
                continue;
        }
    }
    return Status::Success;
}

// =============================================================================
// TREE BUILDER IMPLEMENTATION
// =============================================================================
std::unique_ptr<Sequence> TreeBuilder::Seq(const std::string& name) {
    return std::make_unique<Sequence>(name);
}

std::unique_ptr<Selector> TreeBuilder::Sel(const std::string& name) {
    return std::make_unique<Selector>(name);
}

std::unique_ptr<Action> TreeBuilder::Act(const std::string& name, Action::ActionFunc func) {
    return std::make_unique<Action>(name, func);
}

} // namespace BehaviorTrees
} // namespace Pathfinding