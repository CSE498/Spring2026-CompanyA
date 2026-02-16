#define CATCH_CONFIG_MAIN

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../group_specific_content/Group-14/Group14InitialClasses/EventQueue.hpp"

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

TEST_CASE("PeekNextEvent returns smallest time") {
    EventQueue q;

    q.ScheduleEvent(10, [](){});
    q.ScheduleEvent(5, [](){});

    CHECK(q.PeekNextEvent().time == 5);

    // After peek event, it should still exist
    CHECK(q.Empty() == false);
    CHECK(q.PopNextEvent().time == 5);
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
    CHECK(q.CancelEvent(id1) == false); // cancelled event

    //cancelling id1, the next event should be id2 (time=6)
    CHECK(q.PeekNextEvent().time == 6);
    CHECK(q.PeekNextEvent().id == id2);

    auto e = q.PopNextEvent();
    CHECK(e.time == 6);
    CHECK(e.id == id2);

    CHECK(q.Empty() == true);
}
