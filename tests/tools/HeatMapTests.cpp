#include "../../source/Analyze/HeatMap.hpp"
#include "../../source/core/WorldPosition.hpp"
#include "../../source/tools/ActionLog.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

static int GetAllValidGridValues(std::vector<std::vector<int>> grid_values) {
    int sum = 0;
    for (std::vector<int>& x: grid_values) {
        sum += std::accumulate(x.begin(), x.end(), 0);
    }
    return sum;
}
std::ostringstream output_stream;

// Regular heatmap
TEST_CASE("HeatMap: normal grid/world some invalid values", "[HeatMap]") {
    cse498::ActionLog test_log;
    ;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-1, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, -1}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{1, 11}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{11, 1}, cse498::WorldPosition{0, 0});

    cse498::HeatMap map2(test_log, std::pair<int, int>{5, 5}, std::pair<int, int>{10, 10});
    CHECK(GetAllValidGridValues(map2.OutPutHeatMap(output_stream)) == 2);
    CHECK(map2.GetInvalidInputs().size() == 4);
}

TEST_CASE("HeatMap: large grid/world some invalid values", "[HeatMap]") {
    cse498::ActionLog test_log;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-1, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, -1}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{1, 11}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{11, 1}, cse498::WorldPosition{0, 0});

    cse498::HeatMap map4(test_log, std::pair<int, int>{100, 100}, std::pair<int, int>{20000, 20000});
    CHECK(GetAllValidGridValues(map4.OutPutHeatMap(output_stream)) == 4);
    CHECK(map4.GetInvalidInputs().size() == 2);
}

TEST_CASE("HeatMap: uneven grid/world some invalid values", "[HeatMap]") {
    cse498::ActionLog test_log;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-1, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, -1}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{1, 11}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{11, 1}, cse498::WorldPosition{0, 0});

    cse498::HeatMap map6(test_log, std::pair<int, int>{5, 2}, std::pair<int, int>{10, 5});
    CHECK(GetAllValidGridValues(map6.OutPutHeatMap(output_stream)) == 1);
    CHECK(map6.GetInvalidInputs().size() == 5);
}

TEST_CASE("HeatMap: negative grid/world dimensions throw", "[HeatMap]") {
    cse498::ActionLog test_log;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-1, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, -1}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{1, 11}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{11, 1}, cse498::WorldPosition{0, 0});

    REQUIRE_THROWS_AS(cse498::HeatMap(test_log, std::pair<int, int>{-5, -5}, std::pair<double, double>{-10, -10}),
                      std::invalid_argument);
}

TEST_CASE("HeatMap: Adding second log to Heatmap", "[HeatMap]") {
    cse498::ActionLog test_log;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-1, 4}, cse498::WorldPosition{0, 0});

    cse498::HeatMap map9(test_log, std::pair<int, int>{5, 5}, std::pair<int, int>{10, 10});
    CHECK(GetAllValidGridValues(map9.OutPutHeatMap(output_stream)) == 2);
    CHECK(map9.GetInvalidInputs().size() == 1);

    cse498::ActionLog second_log;

    second_log.LogAction(1, "move", cse498::WorldPosition{9, -1}, cse498::WorldPosition{0, 0});
    second_log.LogAction(1, "move", cse498::WorldPosition{1, 11}, cse498::WorldPosition{0, 0});
    second_log.LogAction(1, "move", cse498::WorldPosition{11, 1}, cse498::WorldPosition{0, 0});

    map9.LogLocations(second_log);
    CHECK(GetAllValidGridValues(map9.OutPutHeatMap(output_stream)) == 2);
    CHECK(map9.GetInvalidInputs().size() == 4);
}

TEST_CASE("HeatMap: positions far outside map bounds", "[HeatMap]") {
    cse498::ActionLog test_log;

    test_log.LogAction(1, "move", cse498::WorldPosition{4, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9, 9}, cse498::WorldPosition{0, 0});

    test_log.LogAction(1, "move", cse498::WorldPosition{1000, 1000}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{-500, 4}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{4, -750}, cse498::WorldPosition{0, 0});
    test_log.LogAction(1, "move", cse498::WorldPosition{9999, -9999}, cse498::WorldPosition{0, 0});

    cse498::HeatMap map10(test_log, std::pair<int, int>{5, 5}, std::pair<int, int>{10, 10});
    CHECK(GetAllValidGridValues(map10.OutPutHeatMap(output_stream)) == 2);
    CHECK(map10.GetInvalidInputs().size() == 4);
}
