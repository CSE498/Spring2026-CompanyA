/*
@author Jacob Bettinger
@date 2026-02-04
@brief Behavior Tree implementation
TODO: Add decorator note, Inverter's later
TODO: Implement an example tree implementation for testing purposes
TODO: Add debugging capabilities
*/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <any>
#include <chrono>
#include <functional>

namespace Pathfinding {
namespace BehaviorTrees {

class Blackboard;
class Node;
class Composite;


// This mainly covers data sharing between nodes
class Blackboard {
public:
    template<typename T>
    void Set(const std::string& key, const T& value) {
        data[key] = value;
    }

    template<typename T>
    T Get(const std::string& key, const T& defaultValue = T{}) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool Has(const std::string& key) const;
    void Remove(const std::string& key);
    void Clear();
    std::vector<std::string> GetKeys() const;

private:
    std::unordered_map<std::string, std::any> data;
};

struct ExecutionContext {
    Blackboard& blackboard;
    
    
    ExecutionContext(Blackboard& bb);
};

// Core interface 
class Node {
public:
    enum class Status { Success, Failure, Running };

    virtual ~Node() = default;
    
    // Core interface - template method pattern
    Status Tick(ExecutionContext& context);
    
    virtual void Reset() = 0;
    virtual std::string GetName() const = 0;

protected:
    // Hook methods for subclasses
    virtual void OnEnter(ExecutionContext& context);
    virtual Status OnUpdate(ExecutionContext& context) = 0;
    virtual void OnExit(ExecutionContext& context, Status status);
};


// Base for nodes that hold ultiple children, I.E. Selector
class Composite : public Node {
public:
    void AddChild(std::unique_ptr<Node> child);
    void Reset() override;
    const std::vector<std::unique_ptr<Node>>& GetChildren() const;

protected:
    std::vector<std::unique_ptr<Node>> children;
    size_t currentChild = 0;
};
// Strategy Pattern that determines actions (No implementation yet but setup for future)
class Action : public Node {
public:
    using ActionFunc = std::function<Status(ExecutionContext&)>;
    
    Action(const std::string& name, ActionFunc action);
    std::string GetName() const override;
    void Reset() override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string name;
    ActionFunc action;
};

// CONCRETE COMPOSITIES (CAN ADD MORE)
class Selector : public Composite {
public:
    explicit Selector(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string name;
};
class Sequence : public Composite {
public:
    explicit Sequence(const std::string& name);
    std::string GetName() const override;

protected:
    Status OnUpdate(ExecutionContext& context) override;

private:
    std::string name;
};


// Helper for building the tree
class TreeBuilder {
public:
    static std::unique_ptr<Sequence> Seq(const std::string& name);
    static std::unique_ptr<Selector> Sel(const std::string& name);
    static std::unique_ptr<Action> Act(const std::string& name, Action::ActionFunc func);

};

} // namespace BehaviorTrees
} // namespace Pathfinding