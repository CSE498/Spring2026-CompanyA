#ifndef HEATMAP
#define HEATMAP

#include <cmath>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>
#include "../tools/ActionLog.hpp"

namespace cse498 {
/**
 * Creates a Heatmap of given entity locations
 * This heatmap handles edge values (values on borders) by putting them
 * in the grid down to the right
 */
class HeatMap {
private:
    std::vector<std::pair<double, double>> m_locations; // Locations to be inputted into the heatmap
    std::vector<std::pair<double, double>>
            m_invalid_locations; // Invalid locations from the inputs based on total grid size
    std::vector<std::vector<int>> m_grid_values; // Values of how often player was inside each grid
    std::pair<int, int> m_ideal_heatmap_size; // Grid size user wants smaller square
    std::pair<double, double> m_map_size; // World map size
    std::pair<double, double> m_single_grid_size; // Internally used to determine how much space in the world map
                                                  // one grid should equate to
    std::vector<std::pair<double, double>>
    ParseActionLog(cse498::ActionLog log); // Private function that parses the action log for its values

public:
    /**
     * Constructs a HeatMap from an action log and map dimensions.
     *
     * @param log The ActionLog containing actions to parse into heatmap locations.
     * @param gridsize The desired heatmap grid size as a pair of integers
     *        representing the number of rows and columns.
     * @param total_gridsize The total world/map size as a pair of doubles
     *        representing the width and height.
     */
    HeatMap(cse498::ActionLog log, std::pair<int, int> gridsize, std::pair<double, double> total_gridsize);

    /**
     * Prints the heatmap in text form to the given output stream.
     *
     * @param output The output stream to write the heatmap to.
     *               Defaults to std::cout.
     * @return A 2D vector containing the valid heatmap grid values.
     */
    std::vector<std::vector<int>> OutPutHeatMap(std::ostream& output = std::cout);

    /**
     * Adds more locations to the heatmap from another action log.
     *
     * @param new_log The ActionLog containing additional actions to add.
     */
    void LogLocations(ActionLog new_log);

    /**
     * Returns all invalid input locations recorded by the heatmap.
     *
     * Invalid inputs are locations that fall outside the map bounds.
     *
     * @return A vector of invalid location coordinate pairs.
     */
    std::vector<std::pair<double, double>> GetInvalidInputs();

    /**
     * Fills the heatmap grid using the currently stored locations.
     *
     * Valid locations are counted into the appropriate grid cells.
     * Invalid locations, such as coordinates outside the map bounds,
     * are removed from the pending locations and stored in the invalid
     * location vector.
     *
     * @return A 2D vector containing the updated heatmap grid values.
     */
    std::vector<std::vector<int>> FillGrid();
};

} // namespace cse498
#endif
