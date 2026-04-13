/**
 * @file ActionMap.hpp
 * @author Ty Maksimowski
 *
 * This class serves as a command registry that maps string action names
 * to callable functions.
 *
 * AI Assistance:
 * Portions of this implementation were developed with assistance from AI tools
 * (ChatGPT) as a brainstorming and learning aid. The assistance was primarily
 * focused on:
 * - Designing the typed action system (supporting functions with arguments
 *   and return values).
 * - Exploring type-erasure patterns using std::any and std::function.
 * - Discussing alternative API designs and improving template structure.
 * All final design decisions, implementation details, and testing
 * were reviewed, modified, and validated by the author (Ty Maksimowski).
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

namespace cse498
{
    class ActionMap
    {
    private:
        /**
         * Represents a single registered action.
         *
         * Kept as a struct instead of mapping directly to std::function to allow
         * for storing metadata (just a description for now maybe more later)
         */
        struct ActionEntry
        {
            std::type_index signature{typeid(void)}; // exact std::function signature we registered with
            std::function<std::any(const std::vector<std::any>&)> invoker; // type-erased call wrapper
            std::optional<std::string> description; // optional UI tooltip text
        };

        // Stores mapping from action names to associated entries.
        std::unordered_map<std::string, ActionEntry> mActions;

        /**
         * Helper to define what counts as a valid name.
         * For now, it is only required to be non-empty.
         *
         * @param name attempting to be entered
         * @return whether name is valid
         */
        static bool IsValidName(std::string_view name)
        {
            return !name.empty();
        }

        /**
         * Helper for looking up an action by name
         * We take string_view for the public API, but keys are stored as std::string.
         */
        auto Find(std::string_view name)
        {
            return mActions.find(std::string(name));
        }

        auto Find(std::string_view name) const
        {
            return mActions.find(std::string(name));
        }


        /**
         * Wrap a typed function (R(Args...)) into a single uniform callable:
         *   invoker(vector<any>) -> any
         *
         * This is the core trick that lets one map store different signatures safely.
         */
        template <typename R, typename... Args>
        static std::function<std::any(const std::vector<std::any>&)>
        MakeInvoker(std::function<R(Args...)> func)
        {
            return [fn = std::move(func)](const std::vector<std::any>& packedArgs) -> std::any
            {
                // If argument count doesn't match, treat it as a failure
                if (packedArgs.size() != sizeof...(Args))
                {
                    throw std::bad_any_cast{};
                }

                return InvokeImpl<R, Args...>(fn, packedArgs, std::index_sequence_for<Args...>{});
            };
        }

        template <typename R, typename... Args, std::size_t... I>
        static std::any InvokeImpl(const std::function<R(Args...)>& func, const std::vector<std::any>& packedArgs,
                                   std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<R>)
            {
                // Cast each packed arg to its expected type, then call the function.
                func(std::any_cast<std::remove_cv_t<std::remove_reference_t<Args>>>(packedArgs[I])...);
                return std::any{}; // void returns an empty any
            }
            else
            {
                // Same cast path, but return the actual value back into std::any.
                R result = func(std::any_cast<std::remove_cv_t<std::remove_reference_t<Args>>>(packedArgs[I])...);
                return std::any{std::move(result)};
            }
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

        /**
         * Returns true if action exists.
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
         * Get the description of an action if there is one
         * @param name Name of the action
         * @return (optional) Description of action
         */
        std::optional<std::string> GetDescription(std::string_view name) const;

        /**
         * Set the description of an action
         * @param name
         * @param description
         * @return True if description is set
         */
        bool SetDescription(std::string_view name, std::optional<std::string> description);

        // -------------------------
        // Typed actions (args + return)
        // -------------------------
        template <typename R, typename... Args>
        bool AddFunction(std::string name,
                         std::function<R(Args...)> func,
                         std::optional<std::string> description = std::nullopt)
        {
            // Catch empty function in debug
            assert(static_cast<bool>(func) && "ActionMap::AddFunction function must not be empty");

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

            auto [it, inserted] = mActions.emplace(std::move(name), std::move(entry));
            (void)it;
            return inserted;
        }

        template <typename R, typename... Args>
        std::optional<R> TriggerTyped(std::string_view name, Args&&... args) const
        {
            static_assert(!std::is_void_v<R>,
                          "Use TriggerVoid<Args...>() for void-returning typed actions.");

            if (!IsValidName(name))
            {
                return std::nullopt;
            }

            auto it = Find(name);
            if (it == mActions.end())
            {
                return std::nullopt; // not found
            }

            using Signature = std::function<R(Args...)>;
            if (it->second.signature != std::type_index(typeid(Signature)))
            {
                return std::nullopt; // signature mismatch
            }

            // Pack args into std::any so invoker can unpack them uniformly.
            std::vector<std::any> packedArgs;
            packedArgs.reserve(sizeof...(Args));
            (packedArgs.emplace_back(std::forward<Args>(args)), ...);

            try
            {
                std::any out = it->second.invoker(packedArgs);
                return std::any_cast<R>(out);
            }
            catch (const std::bad_any_cast&)
            {
                // Bad arg types or wrong arg count.
                return std::nullopt;
            }
        }

        template <typename... Args>
        bool TriggerVoid(std::string_view name, Args&&... args) const
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

            using Signature = std::function<void(Args...)>;
            if (it->second.signature != std::type_index(typeid(Signature)))
            {
                return false;
            }

            std::vector<std::any> packedArgs;
            packedArgs.reserve(sizeof...(Args));
            (packedArgs.emplace_back(std::forward<Args>(args)), ...);

            try
            {
                (void)it->second.invoker(packedArgs);
                return true;
            }
            catch (const std::bad_any_cast&)
            {
                return false;
            }
        }
    };
}