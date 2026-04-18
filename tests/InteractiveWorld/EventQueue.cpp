#include "../../source/tools/InteractiveWorld/EventQueue.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

using namespace cse498;

TEST_CASE("EventQueue starts empty", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    CHECK(q.Empty() == true);

    q.ScheduleEvent(5, []() {});
    CHECK(q.Empty() == false);

    q.PopNextEvent();
    CHECK(q.Empty() == true);
}

TEST_CASE("ScheduleEvent adds events with unique ids", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    auto id1 = q.ScheduleEvent(10, []() {});
    auto id2 = q.ScheduleEvent(5, []() {});

    CHECK(id1 != id2);
    CHECK(q.Empty() == false);
}

TEST_CASE("PeekNextEvent returns smallest time and does not remove event", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    q.ScheduleEvent(10, []() {});
    q.ScheduleEvent(5, []() {});

    CHECK(q.PeekNextEvent().time == 5);

    // After peek, the event should still exist.
    CHECK(q.Empty() == false);

    // Pop the first event (time = 5), now the next should be time = 10.
    CHECK(q.PopNextEvent().time == 5);
    CHECK(q.PeekNextEvent().time == 10);
}

TEST_CASE("PopNextEvent removes events in time order", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    q.ScheduleEvent(10, []() {});
    q.ScheduleEvent(5, []() {});
    q.ScheduleEvent(7, []() {});

    CHECK(q.PopNextEvent().time == 5);
    CHECK(q.PopNextEvent().time == 7);
    CHECK(q.PopNextEvent().time == 10);
    CHECK(q.Empty() == true);
}

TEST_CASE("Events with same time are returned in FIFO order", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    auto id1 = q.ScheduleEvent(5, []() {});
    auto id2 = q.ScheduleEvent(5, []() {});
    auto id3 = q.ScheduleEvent(5, []() {});

    CHECK(q.PopNextEvent().id == id1);
    CHECK(q.PopNextEvent().id == id2);
    CHECK(q.PopNextEvent().id == id3);
    CHECK(q.Empty() == true);
}

TEST_CASE("CancelEvent skips cancelled event and returns next valid event", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    auto id1 = q.ScheduleEvent(5, []() {});
    auto id2 = q.ScheduleEvent(6, []() {});

    CHECK(q.CancelEvent(id1) == true);
    CHECK(q.CancelEvent(id1) == false); // already cancelled

    // Cancelling id1 means the next valid event should be id2 (time = 6).
    CHECK(q.PeekNextEvent().time == 6);
    CHECK(q.PeekNextEvent().id == id2);

    auto e = q.PopNextEvent();
    CHECK(e.time == 6);
    CHECK(e.id == id2);

    CHECK(q.Empty() == true);
}

TEST_CASE("CancelEvent returns false for non-existent id", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;

    // Cancel when queue is empty.
    CHECK(q.CancelEvent(12345) == false);

    // Schedule one event, but cancel a different id.
    auto id1 = q.ScheduleEvent(10, []() {});
    (void) id1;
    CHECK(q.CancelEvent(99999) == false);
}

TEST_CASE("Const EventQueue supports PeekNextEvent and Empty", "[core][InteractiveWorld][EventQueue]") {
    EventQueue q;
    q.ScheduleEvent(3, []() {});
    q.ScheduleEvent(8, []() {});

    const EventQueue& cq = q;

    CHECK(cq.Empty() == false);
    CHECK(cq.PeekNextEvent().time == 3);
}
