/**
 * @file ActionMap.cpp
 * @author Ty Maksimowski
 */

#include "ActionMap.hpp"

namespace cse498
{
    bool ActionMap::RemoveFunction(std::string_view name)
    {
        if (!IsValidName(name))
        {
            return false;
        }

        auto it = Find(name);
        if (it == mActions.end())
        {
            return false;
        }

        // Erase by iterator to avoid a second lookup.
        mActions.erase(it);
        return true;
    }

    std::vector<std::string> ActionMap::ListActions() const
    {
        std::vector<std::string> names;
        names.reserve(mActions.size());

        for (const auto& [name, unused] : mActions)
        {
            (void)unused;
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

        auto oldIt = Find(oldName);
        if (oldIt == mActions.end())
        {
            return false;
        }

        if (Find(newName) != mActions.end())
        {
            return false;
        }

        // Move the entry under the new key, then erase the old key.
        mActions.emplace(std::move(newName), std::move(oldIt->second));
        mActions.erase(oldIt);
        return true;
    }

    std::optional<std::string> ActionMap::GetDescription(std::string_view name) const
    {
        if (!IsValidName(name))
        {
            return std::nullopt;
        }

        auto it = Find(name);
        if (it == mActions.end())
        {
            return std::nullopt;
        }

        // Return a copy of the optional description.
        return it->second.description;
    }

    bool ActionMap::SetDescription(std::string_view name, std::optional<std::string> description)
    {
        if (!IsValidName(name))
        {
            return false;
        }

        auto it = Find(name);
        if (it == mActions.end())
        {
            return false;
        }

        // Overwrite existing description (or clear it).
        it->second.description = std::move(description);
        return true;
    }
}