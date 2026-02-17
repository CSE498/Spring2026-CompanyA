/**
 * EventQueue: store events and always give you the next event to run.
 */

#pragma once

#include <queue>
#include <functional>
#include <unordered_set>
#include <cstdint>
#include <cassert>

namespace cse498 {

  using EventID = std::uint64_t;
  using TimePoint = std::uint64_t;

  struct Event {
    TimePoint time;
    EventID id;
    std::uint64_t order;            // tie-break: smaller order first
    std::function<void()> payload;  // what to do

    //https://stackoverflow.com/questions/19535644/how-to-use-the-priority-queue-stl-for-objects
    bool operator<(const Event& other) const {
      if (time != other.time) return time > other.time;  // smaller time first
      return order > other.order;
    }
  };

  class EventQueue {
  private:
    std::priority_queue<Event> events;
    std::unordered_set<EventID> cancelled;
    EventID next_id = 0;
    std::uint64_t next_order = 0;

    // remove cancelled events at the top
    void CleanTop() {
      while (!events.empty() && cancelled.count(events.top().id) > 0) {
        events.pop();
      }
    }

  public:
    EventID ScheduleEvent(TimePoint t, std::function<void()> payload) {
      Event e;
      e.time = t;
      e.id = next_id++;
      e.order = next_order++;
      e.payload = payload;
      events.push(e);
      return e.id;
    }

    const Event& PeekNextEvent() {
      CleanTop();
      assert((!events.empty() && "PeekNextEvent: empty queue"));
      return events.top();
    }

    Event PopNextEvent() {
      CleanTop();
      assert((!events.empty() && "PopNextEvent: empty queue"));
      Event e = events.top();
      events.pop();
      return e;
    }

    bool CancelEvent(EventID id) {
      return cancelled.insert(id).second;
    }

    bool Empty() {
      CleanTop();
      return events.empty();
    }
  };

}
