/**
 * @file ActionMap.h
 * @author Ty Maksimowski
 *
 * This class serves as a command registry that maps string action names
 * to callable functions.
 */

#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class ActionMap
{
private:
    /**
     * Represents a single registered action.
     *
     * Currently only stores the callable function, but keeping this as a struct instead of mapping directly to the
     * function leaves room to more easily expand what is stored such as descriptions, categories, permissions, etc.
     * without changing the underlying container type.
     */
    struct ActionEntry
    {
        std::function<void()> fn;
        // std::optional<std::string> description; // Potential future metadata
    };

    // Stores mapping from action names to associated entries.
    std::unordered_map<std::string, ActionEntry> mActions;

    /**
     * Helper to define what counts as a valid name. For now it is only required to be non-empty.
     */
    static bool IsValidName(std::string_view name)
    {
        return !name.empty();
    }
public:
    ActionMap() = default;

    // Core API:

    /**
     * Adds a new action mapping a name to a callable.
     */
    bool AddFunction(std::string name, std::function<void()> fn);

    /**
     * Removes an action from the map.
     */
    bool RemoveFunction(std::string_view name);

    /**
     * Triggers an action by name.
     *
     * Note: This is a "fire-and-forget" command call (void()) for v1.
     * If we later need arguments/return values, we can extend the interface.
     */
    bool Trigger(std::string_view name) const;

    /**
     * Lists all currently registered action names.
     *
     * Note: Unordered maps do not guarantee order. If a stable order is needed for
     * UI, callers can sort the returned vector (or we can add a sorted listing).
     */
    std::vector<std::string> ListActions() const;

    // Helpful utilities:

    /**
     * Returns the number of registered actions
     */
    std::size_t Size() const;

    /**
     * Removes all action entries.
     */
    void Clear();

    /**
     * Renames an action key without modifying the stored function
     */
    bool Rename(std::string_view oldName, std::string newName);

    // Depending on where project goes more may be added/modified
    // Stuff for optional metadata about actions for example.

};