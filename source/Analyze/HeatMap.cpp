#include "HeatMap.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

cse498::HeatMap::HeatMap(ActionLog log, std::pair<int, int> ideal_heatmap_size,
                         std::pair<double, double> map_size)
    : m_locations(ParseActionLog(log)),
      m_ideal_heatmap_size(ideal_heatmap_size),
      m_map_size(map_size) {

  const auto [heatmap_rows, heatmap_cols] = m_ideal_heatmap_size;
  const auto [map_width, map_height] = m_map_size;
  auto& [cell_width, cell_height] = m_single_grid_size;

  auto invalid_pair = [](auto a, auto b) { return a <= 0 || b <= 0; };

  // Both the heatmap dimensions and map dimensions must be positive
  // before any cell sizing can be computed
  if (invalid_pair(heatmap_rows, heatmap_cols)) {
    throw std::invalid_argument(
        "HeatMap: ideal_heatmap_size must have positive dimensions");
  }

  if (invalid_pair(map_width, map_height)) {
    throw std::invalid_argument(
        "HeatMap: total_gridsize must have positive dimensions");
  }

  // Each heatmap cell represents this much space in the world map
  cell_width = map_width / heatmap_rows;
  cell_height = map_height / heatmap_cols;

  m_grid_values.resize(heatmap_rows);
  for (std::vector<int>& row : m_grid_values) {
    row.resize(heatmap_cols);
  }
}

std::vector<std::vector<int>>
cse498::HeatMap::OutPutHeatMap(std::ostream& output) {
  const auto [heatmap_rows, heatmap_cols] = m_ideal_heatmap_size;

  // Process any pending locations before printing so the output reflects
  // the current state of the heatmap
  FillGrid();

  output << '\n';
  for (int r = 0; r < heatmap_rows; ++r) {
    for (int c = 0; c < heatmap_cols; ++c) {
      output << "----";
    }
    output << "-" << std::endl;

    for (int c = 0; c < heatmap_cols; ++c) {
      output << "| " << m_grid_values[r][c] << " ";
    }
    output << "|" << std::endl;
  }

  for (int c = 0; c < heatmap_cols; ++c) {
    output << "----";
  }

  output << "-" << std::endl;
  output << "Invalid Locations at: " << std::endl;

  // Print any coordinates that fell outside the configured map bounds
  for (const auto& [x, y] : m_invalid_locations) {
    output << x << "," << y << std::endl;
  }

  return m_grid_values;
}

std::vector<std::vector<int>> cse498::HeatMap::FillGrid() {
  const auto [map_width, map_height] = m_map_size;
  const auto [cell_width, cell_height] = m_single_grid_size;

  // A location is valid if it is inside the map bounds and maps to a
  // non-negative grid index
  auto in_bounds = [map_width, map_height](double x, double y, double grid_x,
                                           double grid_y) {
    return x < map_width && y < map_height &&
           grid_x >= 0 && grid_y >= 0;
  };

  // Consume each pending location once so later calls do not double-count
  // locations that were already added to the heatmap
  for (auto it = m_locations.begin(); it != m_locations.end();) {
    const auto& [x, y] = *it;

    // Convert world coordinates into heatmap grid indices
    double temp_grid_x = std::floor(x / cell_width);
    double temp_grid_y = std::floor(y / cell_height);

    if (in_bounds(x, y, temp_grid_x, temp_grid_y)) {
      m_grid_values[temp_grid_x][temp_grid_y] += 1;
    } else {
      // Keep track of out-of-bounds inputs separately for later inspection
      m_invalid_locations.push_back(*it);
    }

    // Remove processed locations so only newly logged ones remain pending
    it = m_locations.erase(it);
  }

  return m_grid_values;
}

void cse498::HeatMap::LogLocations(ActionLog new_actions) {
  // Append newly parsed locations so they can be processed on the next
  // FillGrid call
  for (std::pair<double, double>& x : ParseActionLog(new_actions)) {
    m_locations.push_back(x);
  }
}

std::vector<std::pair<double, double>> cse498::HeatMap::GetInvalidInputs() {
  return m_invalid_locations;
}

std::vector<std::pair<double, double>>
cse498::HeatMap::ParseActionLog(ActionLog log) {
  std::vector<std::pair<double, double>> locations;
  for (auto& action : log.GetActions()) {
    // Only movement actions contribute coordinates to the heatmap.
    if (action.ActionType == "move") {
      locations.push_back(
          std::pair<double, double>{action.Position.X(), action.Position.Y()});
    }
  }
  return locations;
}