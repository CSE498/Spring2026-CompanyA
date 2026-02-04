/**
 * @file ActionMap.cpp
 * @author Ty Maksimowski
 */

#include "ActionMap.h"

bool ActionMap::AddFunction(std::string name, std::function<void()> fn)
{
    // Catches bad usage during development
    assert(static_cast<bool>(fn) && "ActionMap::AddFunction requires a non-empty function");

    // Runtime validation to keep behavior safe even when asserts are disabled.
    if (!IsValidName(name))
    {
        return false;
    }
    if (!fn)
    {
        return false;
    }

    // Use emplace() because it only inserts if the key does not already exist. This is used to prevent
    // silent overwrites
    auto [it, inserted] = mActions.emplace(std::move(name), ActionEntry{std::move(fn)});
    return inserted;
}

bool ActionMap::RemoveFunction(std::string_view name)
{
    if (!IsValidName(name))
    {
        return false;
    }

    // erase() returns the number of elements removed (0 or 1 in this case)
    return mActions.erase(std::string(name)) > 0;
}

bool ActionMap::Trigger(std::string_view name) const
{
    if (!IsValidName(name))
    {
        return false;
    }

    // Keys are stored as std::string, so this v1 implementation converts string_view to
    // std::string for lookup. This is simple and correct, but if we want to avoid the conversion, we
    // can switch to transparent hashing later.
    auto it = mActions.find(std::string(name));
    if (it == mActions.end())
    {
        return false;
    }

    // Call the action.
    it->second.fn();
    return true;
}

std::vector<std::string> ActionMap::ListActions() const
{
    std::vector<std::string> names;
    names.reserve(mActions.size()); // avoids repeated reallocations

    for (const auto& [name, entry] : mActions)
    {
        (void)entry;
        names.push_back(name);
    }
    return names;
}

std::size_t ActionMap::Size() const
{
    return mActions.size();
}

void ActionMap::Clear()
{
    mActions.clear();
}

bool ActionMap::Rename(std::string_view oldName, std::string newName)
{
    if (!IsValidName(oldName) || !IsValidName(newName))
    {
        return false;
    }

    // Find existing action
    auto oldIt = mActions.find(std::string(oldName));
    if (oldIt == mActions.end())
    {
        return false;
    }

    // Check to not overwrite existing actions
    if (mActions.find(newName) != mActions.end())
    {
        return false;
    }

    // Move the action to the new key and erase old one
    mActions.emplace(std::move(newName), std::move(oldIt->second));
    mActions.erase(oldIt);
    return true;
}