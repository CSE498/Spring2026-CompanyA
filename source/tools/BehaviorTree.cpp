#include "BehaviorTree.hpp"

namespace Pathfinding {
namespace BehaviorTrees {

// ============================================================
// BLACKBOARD IMPLEMENTATION
// ============================================================

bool Blackboard::Has(const std::string &key) const {
  return data.find(key) != data.end();
}

void Blackboard::Remove(const std::string &key) { data.erase(key); }

void Blackboard::Clear() { data.clear(); }

std::vector<std::string> Blackboard::GetKeys() const {
  std::vector<std::string> keys;
  keys.reserve(data.size());
  for (const auto &pair : data) {
    keys.push_back(pair.first);
  }
  return keys;
}

// ============================================================
// EXECUTION CONTEXT
// ============================================================

ExecutionContext::ExecutionContext(Blackboard &bb) : blackboard(bb) {}

// ============================================================
// NODE BASE IMPLEMENTATION
// ============================================================

Node::Status Node::Tick(ExecutionContext &context) {
  OnEnter(context);
  Status status = OnUpdate(context);
  OnExit(context, status);
  return status;
}

void Node::OnEnter(ExecutionContext &) {
  // Default: no-op
}

void Node::OnExit(ExecutionContext &, Status) {
  // Default: no-op
}

// ============================================================
// COMPOSITE
// ============================================================

void Composite::AddChild(std::unique_ptr<Node> child) {
  children.push_back(std::move(child));
}

void Composite::Reset() {
  currentChild = 0;
  for (auto &child : children) {
    child->Reset();
  }
}

const std::vector<std::unique_ptr<Node>> &Composite::GetChildren() const {
  return children;
}

// ============================================================
// DECORATOR
// ============================================================

void Decorator::SetChild(std::unique_ptr<Node> childNode) {
  child = std::move(childNode);
}

void Decorator::Reset() {
  if (child) {
    child->Reset();
  }
}

const std::unique_ptr<Node> &Decorator::GetChild() const { return child; }

// ============================================================
// ACTION
// ============================================================

Action::Action(const std::string &name, ActionFunc action)
    : name(name), action(action) {}

std::string Action::GetName() const { return name; }

void Action::Reset() {
  // Typically stateless
}

Node::Status Action::OnUpdate(ExecutionContext &context) {
  return action(context);
}

// ============================================================
// SELECTOR
// ============================================================

Selector::Selector(const std::string &name) : name(name) {}

std::string Selector::GetName() const { return name; }

Node::Status Selector::OnUpdate(ExecutionContext &context) {
  while (currentChild < children.size()) {
    Status status = children[currentChild]->Tick(context);

    if (status == Status::Success) {
      currentChild = 0;
      return Status::Success;
    }

    if (status == Status::Running)
      return Status::Running;

    currentChild++;
  }

  currentChild = 0;
  return Status::Failure;
}

// ============================================================
// SEQUENCE
// ============================================================

Sequence::Sequence(const std::string &name) : name(name) {}

std::string Sequence::GetName() const { return name; }

Node::Status Sequence::OnUpdate(ExecutionContext &context) {
  while (currentChild < children.size()) {
    Status status = children[currentChild]->Tick(context);

    if (status == Status::Failure) {
      currentChild = 0;
      return Status::Failure;
    }

    if (status == Status::Running)
      return Status::Running;

    currentChild++;
  }

  currentChild = 0;
  return Status::Success;
}

// ============================================================
// INVERT
// ============================================================

Invert::Invert(const std::string &name) : name(name) {}

std::string Invert::GetName() const { return name; }

Node::Status Invert::OnUpdate(ExecutionContext &context) {
  if (!child)
    return Status::Failure;

  Status status = child->Tick(context);

  if (status == Status::Success)
    return Status::Failure;

  if (status == Status::Failure)
    return Status::Success;

  return Status::Running;
}

// ============================================================
// CONTINUALLY REPEAT
// ============================================================

ContinuallyRepeat::ContinuallyRepeat(const std::string &name) : name(name) {}

std::string ContinuallyRepeat::GetName() const { return name; }

Node::Status ContinuallyRepeat::OnUpdate(ExecutionContext &context) {
  if (!child)
    return Status::Running;

  Status status = child->Tick(context);

  if (status == Status::Success || status == Status::Failure) {
    child->Reset();
  }

  return Status::Running;
}

// ============================================================
// TREE BUILDER
// ============================================================

std::unique_ptr<Sequence> TreeBuilder::Seq(const std::string &name) {
  return std::make_unique<Sequence>(name);
}

std::unique_ptr<Selector> TreeBuilder::Sel(const std::string &name) {
  return std::make_unique<Selector>(name);
}

std::unique_ptr<Action> TreeBuilder::Act(const std::string &name,
                                         Action::ActionFunc func) {
  return std::make_unique<Action>(name, func);
}

std::unique_ptr<Invert> TreeBuilder::Inv(const std::string &name) {
  return std::make_unique<Invert>(name);
}

std::unique_ptr<ContinuallyRepeat>
TreeBuilder::Repeat(const std::string &name) {
  return std::make_unique<ContinuallyRepeat>(name);
}

} // namespace BehaviorTrees
} // namespace Pathfinding