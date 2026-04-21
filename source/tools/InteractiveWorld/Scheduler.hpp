/**
 * @file Scheduler.hpp
 * @brief Weighted random choice among registered processes using positive priority weights.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

namespace cse498 {

using ProcessID = int;

class Scheduler {
private:
    // Represents a process with an ID and a priority value where higher priority
    // is more likely to be scheduled
    struct Process {
        ProcessID id;
        float priority;
    };

    // Helper function to find a process by ID; returns iterator to process or
    // mProcesses.end() if not found
    auto FindProcess(ProcessID id) {
        return std::ranges::find_if(mProcesses, [id](const Process& p) { return p.id == id; });
    }

    // Const version of FindProcess for use in const methods
    auto FindProcess(ProcessID id) const {
        return std::ranges::find_if(mProcesses, [id](const Process& p) { return p.id == id; });
    }

    std::vector<Process> mProcesses; // List of processes with their priorities
    std::mt19937 mRng{std::random_device{}()}; // Random number generator for
                                               // weighted random selection

public:
    // Add a process with an associated priority
    void AddProcess(ProcessID id, float priority) {
        assert(priority > 0.0f && "Scheduler priority must be positive");

        // Prevent duplicate IDs
        assert(!HasProcess(id) && "Process already exists in Scheduler");

        mProcesses.push_back({id, priority});
    }

    // Returns the ID of the next process to schedule
    ProcessID Next() {
        assert(!mProcesses.empty() && "Scheduler is empty");

        const float totalPriority =
                std::accumulate(mProcesses.begin(), mProcesses.end(), 0.0f,
                                [](float total, const Process& process) { return total + process.priority; });

        assert(totalPriority > 0.0f && "Total priority must be > 0");

        std::uniform_real_distribution<float> dist(0.0f, totalPriority);
        float r = dist(mRng);

        float cumulative = 0.0f;
        for (const auto& [id, priority]: mProcesses) {
            cumulative += priority;
            if (r <= cumulative) {
                return id;
            }
        }

        // Floating-point safety fallback, just in case
        return mProcesses.back().id;
    }

    // Check whether a process exists
    bool HasProcess(ProcessID id) const { return FindProcess(id) != mProcesses.end(); }

    // Remove a process; returns false if not found
    bool RemoveProcess(ProcessID id) {
        auto it = FindProcess(id);
        if (it == mProcesses.end()) {
            return false;
        }

        mProcesses.erase(it);
        return true;
    }

    // Update the priority of an existing process
    void UpdatePriority(ProcessID id, float newPriority) {
        assert(newPriority > 0.0f && "Scheduler priority must be positive");

        auto it = FindProcess(id);
        assert(it != mProcesses.end() && "Process not found in Scheduler");

        it->priority = newPriority;
    }

    // Remove all processes
    void Clear() { mProcesses.clear(); }

    // Check if there are no processes in the scheduler
    bool Empty() const { return mProcesses.empty(); }
};

} // namespace cse498
