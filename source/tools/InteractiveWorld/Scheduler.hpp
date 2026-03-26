/**
 * Scheduler: Schedules a set of processes based on a "priority" heuristic and
 * returns the next process to run using a weighted random selection approach
 * where higher priority processes are more likely to be selected.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <random>
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
  // processes.end() if not found
  auto FindProcess(ProcessID id) {
    return std::find_if(processes.begin(), processes.end(),
                        [id](const Process &p) { return p.id == id; });
  }

  // Const version of FindProcess for use in const methods
  auto FindProcess(ProcessID id) const {
    return std::find_if(processes.begin(), processes.end(),
                        [id](const Process &p) { return p.id == id; });
  }

  std::vector<Process> processes; // List of processes with their priorities
  std::mt19937 rng{std::random_device{}()}; // Random number generator for
                                            // weighted random selection

public:
  // Add a process with an associated priority
  void AddProcess(ProcessID id, float priority) {
    assert(priority > 0.0f && "Scheduler priority must be positive");

    // Prevent duplicate IDs
    assert(!HasProcess(id) && "Process already exists in Scheduler");

    processes.push_back({id, priority});
  }

  // Returns the ID of the next process to schedule
  ProcessID Next() {
    assert(!processes.empty() && "Scheduler is empty");

    // Determines next process to run based on weighted random selection using
    // priorities
    float totalPriority = 0.0f;
    for (const auto &p : processes) {
      totalPriority += p.priority;
    }

    // Checks that total priority is positive
    assert(totalPriority > 0.0f && "Total priority must be > 0");

    std::uniform_real_distribution<float> dist(0.0f, totalPriority);
    float r = dist(rng);

    // Iterate through processes and return the first one where the cumulative
    // priority exceeds r
    float cumulative = 0.0f;
    for (const auto &p : processes) {
      cumulative += p.priority;
      if (r <= cumulative) {
        return p.id;
      }
    }

    // Floating-point safety fallback, just in case
    return processes.back().id;
  }

  // Check whether a process exists
  bool HasProcess(ProcessID id) const {
    return FindProcess(id) != processes.end();
  }

  // Remove a process; returns false if not found
  bool RemoveProcess(ProcessID id) {
    auto it = FindProcess(id);
    if (it == processes.end()) {
      return false;
    }

    processes.erase(it);
    return true;
  }

  // Update the priority of an existing process
  void UpdatePriority(ProcessID id, float newPriority) {
    assert(newPriority > 0.0f);

    auto it = FindProcess(id);
    assert(it != processes.end() && "Process not found in Scheduler");

    it->priority = newPriority;
  }

  // Remove all processes
  void Clear() { processes.clear(); }

  // Check if there are no processes in the scheduler
  bool Empty() const { return processes.empty(); }
};

} // namespace cse498