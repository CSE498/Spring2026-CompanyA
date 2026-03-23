/**
 * @file ActionMap.hpp
 * @author Ty Maksimowski
 *
 * This class serves as a command registry that maps string action names
 * to callable functions.
 */

#pragma once

#include <any>          // std::any for type-erased argument passing/return
#include <cassert>      // assert() for programmer mistakes in debug
#include <functional>   // std::function
#include <optional>     // std::optional for description and return
#include <string>       // std::string keys
#include <string_view>  // non-owning string for params
#include <type_traits>  // std::is_void_v, remove_reference_t, etc
#include <typeindex>    // std::type_index for signature checking
#include <unordered_map>
#include <utility>
#include <vector>
#include <expected>

namespace cse498
{
    class ActionMap
    {
    private:

        enum class InvokeError
        {
            WrongArgCount,
            WrongArgType
        };

        /**
         * Represents a single registered action.
         *
         * Stored as a struct rather than mapping directly to std::function so the
         * action can also carry metadata such as an optional description.
         */
        struct ActionEntry
        {
            std::type_index signature{typeid(void)}; // exact std::function signature we registered with
            std::function<std::expected<std::any, InvokeError>(const std::vector<std::any>&)> invoker; // type-erased call wrapper
            std::optional<std::string> description; // optional UI tooltip text
        };

        // Stores mapping from action names to associated entries.
        std::unordered_map<std::string, ActionEntry> mActions;

        /**
         * Returns whether a name is valid for an action.
         * 
         * For now, it is only required to be non-empty.
         *
         * @param name to validate
         * @return True if name is valid, false otherwise
         */
        static bool IsValidName(std::string_view name)
        {
            return !name.empty();
        }

        /**
         * Looks up an action by name
         * The public API accepts std::string_view, while the map stores std::string keys.
         * 
         * @param name Name of the action to find
         * @return Iterator to the found action, or mActions.end() if not found
         */
        auto Find(std::string_view name)
        {
            return mActions.find(std::string(name));
        }

        auto Find(std::string_view name) const
        {
            return mActions.find(std::string(name));
        }

        template <typename T>
        static const std::remove_cvref_t<T>* AnyPtr(const std::any& value)
        {
            return std::any_cast<std::remove_cvref_t<T>>(&value);
        }

        /**
         * Wrap a typed function (R(Args...)) into a single uniform callable:
         *   invoker(vector<any>) -> any
         *
         * This allows actions with different signatures to be stored in the same map.
         */
        template <typename R, typename... Args>
        static std::function<std::expected<std::any, InvokeError>(const std::vector<std::any>&)>
        MakeInvoker(std::function<R(Args...)> func)
        {
            return [fn = std::move(func)](const std::vector<std::any>& packedArgs)
                -> std::expected<std::any, InvokeError>
            {
                // If argument count doesn't match, treat it as a failure
                if (packedArgs.size() != sizeof...(Args))
                {
                    return std::unexpected(InvokeError::WrongArgCount);
                }

                return InvokeImpl<R, Args...>(fn, packedArgs, std::index_sequence_for<Args...>{});
            };
        }

        template <typename R, typename... Args, std::size_t... I>
        static std::expected<std::any, InvokeError>
        InvokeImpl(const std::function<R(Args...)>& func,
                   const std::vector<std::any>& packedArgs,
                   std::index_sequence<I...>)
        {
            if ((((AnyPtr<Args>(packedArgs[I])) == nullptr) || ...))
            {
                return std::unexpected(InvokeError::WrongArgType);
            }

            if constexpr (std::is_void_v<R>)
            {
                func((*AnyPtr<Args>(packedArgs[I]))...);
                return std::any{}; // void returns an empty any
            }
            else
            {
                R result = func((*AnyPtr<Args>(packedArgs[I]))...);
                return std::any{std::move(result)};
            }
        }

        /**
         * Validates a trigger request, looks up action, checks expected signature, and packs arguments into vector<any>.
         *
         * @param name Name of the action to invoke
         * @param args Arguments for the action
         * @return A pair containing an iterator to the action and a vector of packed arguments, or {mActions.end(), {}} if validation or lookup fails
         */
        template <typename Signature, typename... Args>
        auto PrepareInvocation(std::string_view name, Args&&... args) const
             -> std::pair<typename std::unordered_map<std::string, ActionEntry>::const_iterator, std::vector<std::any>>
        {
            auto endIt = mActions.end();

            if (!IsValidName(name))
            {
                return {endIt, {}};
            }

            auto it = Find(name);
            if (it == endIt)
            {
                return {endIt, {}};
            }

            if (it->second.signature != std::type_index(typeid(Signature)))
            {
                return {endIt, {}};
            }

            std::vector<std::any> packedArgs;
            packedArgs.reserve(sizeof...(Args));
            (packedArgs.emplace_back(std::forward<Args>(args)), ...);

            return {it, std::move(packedArgs)};
        }

    public:
        ActionMap() = default;

        // Core API:

        /**
         * Registers a basic void() action mapping a name to a callable.
         * Returns false if invalid name, empty function, or name already exists.
         *
         * Description is optional metadata used by UI/Web for tooltips, menus, etc.
         */
        bool AddFunction(std::string name, std::function<void()> func,
                         std::optional<std::string> description = std::nullopt)
        {
            // Forward to the typed version to maintain one implementation
            return AddFunction<void>(std::move(name), std::move(func), std::move(description));
        }

        /**
         * Removes an action from the map.
         */
        bool RemoveFunction(std::string_view name);

        /**
         * Trigger a void() action with no args.
         */
        bool Trigger(std::string_view name) const
        {
            return TriggerVoid<>(name);
        }

        /**
         * Trigger a void action that takes arguments.
         * Ex: Trigger<void>("SetHP", 100)
         */
        template <typename R = void, typename... Args>
        std::enable_if_t<std::is_void_v<R>, bool>
        Trigger(std::string_view name, Args&&... args) const
        {
            return TriggerVoid<Args...>(name, std::forward<Args>(args)...);
        }

        /**
         * Trigger an action that returns a value.
         * Ex: auto sum = Trigger<int>("Add", 2, 5)
         */
        template <typename R, typename... Args>
        std::enable_if_t<!std::is_void_v<R>, std::optional<R>>
        Trigger(std::string_view name, Args&&... args) const
        {
            return TriggerTyped<R, Args...>(name, std::forward<Args>(args)...);
        }

        /**
         * Lists all currently registered action names.
         *
         * Note: Unordered maps do not guarantee order. If a stable order is needed for
         * UI, callers can sort the returned vector (or we can add a sorted listing).
         * 
         * @return vector of all registered action names
         */
        std::vector<std::string> ListActions() const;

        // Helpful utilities:

        /**
         * Returns the number of registered actions
         * 
         * @return count of registered actions
         */
        std::size_t Size() const;

        /**
         * Removes all action entries.
         */
        void Clear();

        /**
         * Renames an action key without modifying the stored function
         * 
         * @param oldName current name of the action
         * @param newName new name to assign to the action
         * @return true if rename succeeded, false if oldName doesn't exist, newName is invalid, or newName already exists
         */
        bool Rename(std::string_view oldName, std::string newName);

        /**
         * Returns true if action exists.
         * Note: This is not strictly necessary since Trigger* can be used to check existence, but it can be more efficient if caller only needs to check presence without triggering.
         * 
         * @param name Name of the action to check
         * @return true if action exists, false if it doesn't exist or name is invalid
         */
        bool Has(std::string_view name) const
        {
            if (!IsValidName(name))
            {
                return false;
            }
            return Find(name) != mActions.end();
        }

        /**
         * Get the description of an action, if there is one.
         * 
         * @param name Name of the action
         * @return The description if action exists and has one, otherwise std::nullopt
         */
        std::optional<std::string> GetDescription(std::string_view name) const;

        /**
         * Set or clear the description of an action.
         * 
         * @param name Name of the action
         * @param description New description to set, or std::nullopt to clear
         * @return True if action exists and description was updated, otherwise false
         */
        bool SetDescription(std::string_view name, std::optional<std::string> description);

        // -------------------------
        // Typed actions (args + return)
        // -------------------------

        /**
         * Adds a new function to the action map.
         *
         * @param name Name of the action
         * @param func Function to add
         * @param description Optional description for the action
         * @return true if function was added, false if name is invalid or function is empty
         */
        template <typename R, typename... Args>
        bool AddFunction(std::string name,
                         std::function<R(Args...)> func,
                         std::optional<std::string> description = std::nullopt)
        {
            // Catch empty function in debug
            assert(func && "ActionMap::AddFunction function must not be empty");

            if (!IsValidName(name) || !func)
            {
                return false;
            }

            // Signature prevents calling something with wrong template args
            using Signature = std::function<R(Args...)>;

            ActionEntry entry;
            // Store the signature to guard Trigger* calls later.
            entry.signature = std::type_index(typeid(Signature));
            entry.invoker = MakeInvoker<R, Args...>(std::move(func));
            entry.description = std::move(description);

            return mActions.emplace(std::move(name), std::move(entry)).second;
        }

        template <typename R, typename... Args>
        std::optional<R> TriggerTyped(std::string_view name, Args&&... args) const
        {
            static_assert(!std::is_void_v<R>,
                          "Use TriggerVoid<Args...>() for void-returning typed actions.");

            using Signature = std::function<R(Args...)>;
            auto [it, packedArgs] = PrepareInvocation<Signature>(name, std::forward<Args>(args)...);

            if (it == mActions.end())
            {
                return std::nullopt; // not found
            }

            auto out = it->second.invoker(packedArgs);
            if (!out)
            {
                return std::nullopt;
            }

            if (auto result = std::any_cast<R>(&out.value()))
            {
                return *result;
            }
            return std::nullopt;
        }

        template <typename... Args>
        bool TriggerVoid(std::string_view name, Args&&... args) const
        {
            using Signature = std::function<void(Args...)>;
            auto [it, packedArgs] = PrepareInvocation<Signature>(name, std::forward<Args>(args)...);

            if (it == mActions.end())
            {
                return false;
            }
            return it->second.invoker(packedArgs).has_value();
        }
    };
}