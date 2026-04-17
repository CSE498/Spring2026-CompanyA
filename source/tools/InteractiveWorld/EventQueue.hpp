/**
 * EventQueue: store events and always give you the next event to run.
 *
 * Rules:
 * 1) Smaller time runs first.
 * 2) If time ties, earlier inserted event runs first (FIFO).
 * 3) Cancel is "lazy": we mark an id as cancelled and skip it when it reaches
 *    the top.
 *
 * TimePoint unit:
 * - TimePoint is tick-based time represented as uint64_t.
 * - Other modules should pass logical world ticks, not wall-clock milliseconds.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <queue>
#include <unordered_set>
#include <utility>

namespace cse498 {

using EventID = std::uint64_t;
using TimePoint = std::uint64_t; // Tick-based time unit.

/// @brief Single scheduled event stored in the queue.
struct Event {
    TimePoint time; // Tick when the event should run.
    EventID id; // Unique identifier.
    std::uint64_t order; // Insertion order (tie-break).
    std::function<void()> payload; // Action to execute when triggered.

    // priority_queue puts "largest" at the top.
    // Reverse comparison so that the smallest time appears first.
    bool operator<(const Event& other) const {
        if (time != other.time) {
            return time > other.time;
        }
        return order > other.order;
    }
};

class EventQueue {
private:
    // mutable is used because const query functions such as PeekNextEvent()
    // and Empty() still need to lazily discard cancelled events at the top.
    mutable std::priority_queue<Event> m_events;
    mutable std::unordered_set<EventID> m_cancelled_ids;
    mutable std::unordered_set<EventID> m_active_ids;

    EventID m_next_id = 0;
    std::uint64_t m_next_order = 0;

    /// @brief Remove cancelled events sitting at the top of the heap.
    void CleanTop() const {
        while (!m_events.empty()) {
            const Event& top = m_events.top();

            if (m_cancelled_ids.count(top.id) == 0) {
                break;
            }

            const EventID cancelled_id = top.id;
            m_events.pop();
            m_cancelled_ids.erase(cancelled_id);
            m_active_ids.erase(cancelled_id);
        }
    }

public:
    /// @brief Schedule a new event at tick t and return its ID.
    EventID ScheduleEvent(TimePoint t, std::function<void()> payload) {
        assert(payload && "ScheduleEvent: payload must not be empty");

        Event e;
        e.time = t;
        e.id = m_next_id++;
        e.order = m_next_order++;
        e.payload = std::move(payload);

        m_events.push(e);
        m_active_ids.insert(e.id);
        return e.id;
    }

    /// @brief Peek the next valid event without removing it.
    /// @note Programmer error to peek an empty queue.
    const Event& PeekNextEvent() const {
        CleanTop();
        assert((!m_events.empty() && "PeekNextEvent: empty queue"));
        return m_events.top();
    }

    /// @brief Pop and return the next valid event.
    /// @note Programmer error to pop an empty queue.
    Event PopNextEvent() {
        CleanTop();
        assert((!m_events.empty() && "PopNextEvent: empty queue"));

        Event e = m_events.top();
        m_events.pop();

        m_active_ids.erase(e.id);
        m_cancelled_ids.erase(e.id);

        return e;
    }

    /// @brief Cancel an active event by ID.
    /// @return true if cancellation happened now, false if the id was inactive
    ///         or already cancelled.
    bool CancelEvent(EventID id) {
        if (m_active_ids.count(id) == 0) {
            return false;
        }

        return m_cancelled_ids.insert(id).second;
    }

    /// @brief Check whether the queue has any valid remaining events.
    bool Empty() const {
        CleanTop();
        return m_events.empty();
    }
};

} // namespace cse498
