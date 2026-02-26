/**
 * EventQueue: store events and always give you the next event to run.
 *
 * Rules:
 * 1) Smaller time runs first.
 * 2) If time ties, earlier inserted event runs first (FIFO).
 * 3) Cancel is "lazy": we mark an id as cancelled and skip it when it reaches the top.
 */

#pragma once

#include <queue>
#include <functional>
#include <unordered_set>
#include <cstdint>
#include <cassert>
#include <utility>

namespace cse498 {

  using EventID   = std::uint64_t;
  using TimePoint = std::uint64_t;

  /// @brief Single scheduled event stored in the queue.
  struct Event {
    TimePoint time;                 // when to run
    EventID id;                     // unique identifier
    std::uint64_t order;            // insertion order (tie-break)
    std::function<void()> payload;  // what to do

    // priority_queue puts "largest" at the top.
    // We reverse comparison so that the smallest time appears at the top.
    //https://stackoverflow.com/questions/19535644/how-to-use-the-priority-queue-stl-for-objects
    bool operator<(const Event& other) const {
      if (time != other.time) return time > other.time;   // smaller time first
      return order > other.order;                         // FIFO if same time
    }
  };

  class EventQueue {
  private:
    // Main event heap (top = next event)
    std::priority_queue<Event> m_events;

    // IDs that are marked cancelled (lazy cancellation)
    std::unordered_set<EventID> m_cancelled_ids;

    // IDs that are currently scheduled and not yet popped/removed
    std::unordered_set<EventID> m_active_ids;

    // Next id/order counters
    EventID m_next_id = 0;
    std::uint64_t m_next_order = 0;

    /// @brief Remove cancelled events sitting at the top of the heap.
    /// If we pop a cancelled event, we also clean its id from sets.
    void CleanTop() {
      while (!m_events.empty()) {
        const Event& top = m_events.top();

        // If top event isn't cancelled, stop cleaning.
        if (m_cancelled_ids.count(top.id) == 0) break;

        // Top event is cancelled -> remove it from heap
        const EventID cancelled_id = top.id;
        m_events.pop();

        // Fully clean the id
        m_cancelled_ids.erase(cancelled_id);
        m_active_ids.erase(cancelled_id);
      }
    }

  public:
    /// @brief Schedule a new event at time t. Returns the new event's ID.
    EventID ScheduleEvent(TimePoint t, std::function<void()> payload) {
      Event e;
      e.time = t;
      e.id = m_next_id++;
      e.order = m_next_order++;
      e.payload = std::move(payload);

      m_events.push(e);
      m_active_ids.insert(e.id);
      return e.id;
    }

    /// @brief Peek the next event (does not remove it).
    /// @note Programmer error to peek an empty queue.
    const Event& PeekNextEvent() {
      CleanTop();
      assert((!m_events.empty() && "PeekNextEvent: empty queue"));
      return m_events.top();
    }

    /// @brief Pop the next event (removes and returns it).
    /// @note Programmer error to pop an empty queue.
    Event PopNextEvent() {
      CleanTop();
      assert((!m_events.empty() && "PopNextEvent: empty queue"));

      Event e = m_events.top();
      m_events.pop();

      // Event is no longer active once popped.
      m_active_ids.erase(e.id);

      // If it was previously marked cancelled (rare case), clean that too.
      m_cancelled_ids.erase(e.id);

      return e;
    }

    /// @brief Cancel an active event by ID.
    /// @return true if cancellation happened now, false if:
    ///         - id was not active, OR
    ///         - id was already cancelled.
    bool CancelEvent(EventID id) {
      // User error: cancelling an id that doesn't exist / not active
      if (m_active_ids.count(id) == 0) return false;

      // Mark as cancelled (lazy). If already cancelled, insert will fail.
      return m_cancelled_ids.insert(id).second;
    }

    /// @brief Check if there are any valid (non-cancelled) events left.
    bool Empty() {
      CleanTop();
      return m_events.empty();
    }
  };

}