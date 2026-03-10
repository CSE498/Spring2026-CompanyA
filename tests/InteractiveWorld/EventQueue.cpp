#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/InteractiveWorld/EventQueue.hpp"

using namespace cse498;

TEST_CASE("Empty Event") {
    EventQueue q;

    CHECK(q.Empty() == true);

    q.ScheduleEvent(5, [](){});
    CHECK(q.Empty() == false);

    q.PopNextEvent();
    CHECK(q.Empty() == true);
}

TEST_CASE("ScheduleEvent adds events") {
    EventQueue q;

    auto id1 = q.ScheduleEvent(10, [](){});
    auto id2 = q.ScheduleEvent(5, [](){});

    CHECK(id1 != id2);
    CHECK(q.Empty() == false);
}

TEST_CASE("PeekNextEvent returns smallest time (and next is correct too)") {
    EventQueue q;

    q.ScheduleEvent(10, [](){});
    q.ScheduleEvent(5, [](){});

    CHECK(q.PeekNextEvent().time == 5);

    // After peek, it should still exist
    CHECK(q.Empty() == false);

    // Pop the 1st (time=5), now the next should be time=10
    CHECK(q.PopNextEvent().time == 5);
    CHECK(q.PeekNextEvent().time == 10);
}

TEST_CASE("PopNextEvent removes in order") {
    EventQueue q;

    q.ScheduleEvent(10, [](){});
    q.ScheduleEvent(5, [](){});
    q.ScheduleEvent(7, [](){});

    CHECK(q.PopNextEvent().time == 5);
    CHECK(q.PopNextEvent().time == 7);
    CHECK(q.PopNextEvent().time == 10);
    CHECK(q.Empty() == true);
}

TEST_CASE("CancelEvent skips cancelled event and returns next event (id2)") {
    EventQueue q;

    auto id1 = q.ScheduleEvent(5, [](){});
    auto id2 = q.ScheduleEvent(6, [](){});

    CHECK(q.CancelEvent(id1) == true);
    CHECK(q.CancelEvent(id1) == false); // already cancelled

    // cancelling id1 => next should be id2 (time=6)
    CHECK(q.PeekNextEvent().time == 6);
    CHECK(q.PeekNextEvent().id == id2);

    auto e = q.PopNextEvent();
    CHECK(e.time == 6);
    CHECK(e.id == id2);

    CHECK(q.Empty() == true);
}

TEST_CASE("CancelEvent returns false for non-existent id (edge case)") {
    EventQueue q;

    // cancel when queue is empty
    CHECK(q.CancelEvent(12345) == false);

    // schedule one event, but cancel a different id
    auto id1 = q.ScheduleEvent(10, [](){});
    (void)id1;
    CHECK(q.CancelEvent(99999) == false);
}