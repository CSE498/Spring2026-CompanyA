/*
@author Jacob Bettinger
@date 2026-02-04

Note
- Used AI assistance for part of the documentation due to notes during review

Implements:
- Blackboard for shared node data
- Template Method pattern via Node::Tick()
- Composite pattern (Selector, Sequence)
- Decorator pattern (Invert, ContinuallyRepeat)
- Strategy pattern (Action via std::function)
*/

#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498 {
namespace BehaviorTrees {

// ============================================================
// BLACKBOARD
// ============================================================

// Shared key-value storage used by nodes during execution.
// Uses std::any to support arbitrary data types.
class Blackboard {
public:
    // Store a typed value under a key.
    template<typename T>
    void Set(const std::string& key, const T& value) {
        mData[key] = value;
    }

    // Retrieve a typed value.
    // Returns defaultValue if key not found or type mismatch.
    template<typename T>
    T Get(const std::string& key, const T& defaultValue = T{}) const {
        auto it = mData.find(key);
        if (it != mData.end()) {
            const T* p = std::any_cast<T>(&it->second);
            if (p)
                return *p;
        }
        return defaultValue;
    }

    bool Has(const std::string& key) const;
    /**
     * Safe removal. If key doesn't exist then doesn't do anything
     * @param key - tries to remove this key from unordered_map
     */
    void Remove(const std::string& key);
    void Clear();
    std::vector<std::string> GetKeys() const;

private:
    std::unordered_map<std::string, std::any> mData;
};

// ============================================================
// EXECUTION CONTEXT
// ============================================================

// Runtime context passed into every node Tick().
// Currently only exposes the Blackboard but is extensible.
struct ExecutionContext {
    Blackboard& mBlackboard;
    explicit ExecutionContext(Blackboard& bb);
};

// ============================================================
// CORE NODE INTERFACE
// ============================================================

// Base class for all behavior tree nodes.
// Uses Template Method pattern via Tick().
class Node {
public:
    enum class Status { Success, Failure, Running };

    virtual ~Node() = default;

    // Main execution entry point.
    // Calls OnEnter → OnUpdate → OnExit.
    Status Tick(ExecutionContext& context);

    // Reset node state.
    virtual void Reset() = 0;

    // Human-readable name (useful for debugging).
    virtual std::string GetName() const = 0;

protected:
    virtual void OnEnter(ExecutionContext& context);
    virtual Status OnUpdate(ExecutionContext& context) = 0;
    virtual void OnExit(ExecutionContext& context, Status status);
};

// ============================================================
// COMPOSITE BASE (Multiple Children)
// ============================================================

// Base class for nodes that manage multiple children.
class Composite : public Node {
public:
    void AddChild(std::unique_ptr<Node> child);
    void Reset() override;

    // Read-only access to children.
    const std::vector<std::unique_ptr<Node>>& GetChildren() const;

protected:
    std::vector<std::unique_ptr<Node>> mChildren;
    size_t mCurrentChild = 0; // Tracks active child during execution
};

// ============================================================
// DECORATOR BASE (Single Child)
// ============================================================

// Base class for nodes that wrap exactly one child.
class Decorator : public Node {
public:
    void SetChild(std::unique_ptr<Node> child);
    void Reset() override;

    const std::unique_ptr<Node>& GetChild() const;

protected:
    std::unique_ptr<Node> mChild;
};

// ============================================================
// ACTION NODE (Leaf)
// ============================================================

// Leaf node that executes a function.
// Uses Strategy pattern for flexible behavior definition.
class Action : public Node {
public:
    using ActionFunc = std::function<Status(ExecutionContext&)>;

    Action(const std::string& name, ActionFunc action);
    std::string GetName() const override;
    void Reset() override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string mName;
    ActionFunc mAction;
};

// ============================================================
// COMPOSITE IMPLEMENTATIONS
// ============================================================

// Selector:
// Runs children in order until one succeeds.
// Fails only if all children fail.
class Selector : public Composite {
public:
    explicit Selector(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string mName;
};

// Sequence:
// Runs children in order until one fails.
// Succeeds only if all children succeed.
class Sequence : public Composite {
public:
    explicit Sequence(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string mName;
};

// ============================================================
// DECORATOR IMPLEMENTATIONS
// ============================================================

// Invert:
// Flips Success ↔ Failure.
// Running passes through unchanged.
class Invert : public Decorator {
public:
    explicit Invert(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string mName;
};

// ContinuallyRepeat:
// Continuously executes its child.
// Always returns Running.
class ContinuallyRepeat : public Decorator {
public:
    explicit ContinuallyRepeat(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string mName;
};

// ============================================================
// TREE BUILDER (Factory Helpers)
// ============================================================

// Convenience factory methods for constructing trees.
class TreeBuilder {
public:
    /**
     * Runs children in order until one fails. Succeeds only if all children succeed.
     * (Manages Multiple children aka a composite)
     * @param name - name of node
     * @return seq
     */
    static std::unique_ptr<Sequence> Seq(const std::string& name);
    /**
     *  Runs children in order until one succeeds. Fails only if all children fail.
     * (Manages Multiple children aka a composite)
     * @param name - name of node
     * @return selector node
     */
    static std::unique_ptr<Selector> Sel(const std::string& name);
    /**
     * Leaf node to execute function on reaching it
     * @param name
     * @param func - func taking in execution context (blackboard) and return status: suc, fail, running.
     * @return
     */
    static std::unique_ptr<Action> Act(const std::string& name, Action::ActionFunc func);
    /**
     * Flips Success ↔ Failure. Running passes through unchanged.
     * (single node control aka Decorator)
     * @param name - name of node
     * @return inversion node.
     */
    static std::unique_ptr<Invert> Inv(const std::string& name);
    /**
     * Continuously executes its child. Always returns Running.
     * (single node control aka Decorator)
     * @param name - node name
     * @return
     */
    static std::unique_ptr<ContinuallyRepeat> Repeat(const std::string& name);
};

} // namespace BehaviorTrees
} // namespace cse498
