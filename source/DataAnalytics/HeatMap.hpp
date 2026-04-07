#ifndef HEATMAP
#define HEATMAP

#include "../tools/ActionLog.hpp"
#include <cmath>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

namespace cse498 {
/**
 * Creates a Heatmap of given entity locations
 * This heatmap handles edge values (values on borders) by putting them
 * in the grid down to the right
 */
class HeatMap {
private:
  std::vector<std::pair<double, double>> m_locations;   // Locations to be inputted into the heatmap
  std::vector<std::pair<double, double>> m_invalid_locations;  // Invalid locations from the inputs based on total grid size
  std::vector<std::vector<int>> m_grid_values;   // Values of how often player was inside each grid
  std::pair<int, int> m_ideal_heatmap_size;   // Grid size user wants smaller square
  std::pair<double, double> m_map_size;   // World map size
  std::pair<double, double> m_single_grid_size;   // Internally used to determine how much space in the world map one grid should equate to
  std::vector<std::pair<double, double>> ParseActionLog(cse498::ActionLog log); //Private function that parses the action log for its values

public:
  /** Constructor
   * ActionLog of all actions to be parsed
   * Gridsize you want to see pair<int,int>
   * Worldmap size pair<double,double>
   */
  HeatMap(cse498::ActionLog log, std::pair<int, int> gridsize,
          std::pair<double, double> total_gridsize);
  /**
   * Prints the heatmap out in text form
   * Takes stream to output somewhere else, deafult std::cout
   * Returns vector of valid values
   */
  std::vector<std::vector<int>> OutPutHeatMap(std::ostream &output = std::cout);
  /**
   * Add more locations to the heatmap
   * ActionLog (other action logs if more are to be added)
   */
  void LogLocations(ActionLog new_log);
  /**
   * Outputs any invalid inputs (i.e. character glitches out of map)
   */
  std::vector<std::pair<double, double>> GetInvalidInputs();

/**
 * Fills the grid values based on the locations and grid sizes,
 * Removes invalid locations (locations from outside the inputted map size)
 * from the heatmap and adds them to the invalid location vector
 */
  std::vector<std::vector<int>> FillGrid();  
};

} // namespace cse498
#endif
