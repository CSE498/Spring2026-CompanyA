#include "HeatMap.hpp"

#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

cse498::HeatMap::HeatMap(ActionLog log, std::pair<int, int> ideal_heatmap_size,
                         std::pair<double, double> map_size) : 
  m_locations(ParseActionLog(log)),
  m_ideal_heatmap_size(ideal_heatmap_size),
  m_map_size(map_size)
  {

  auto invalid_pair = [](auto a, auto b) { return a <= 0 || b <= 0; };

  auto invalid_pair = [](auto a, auto b) {
    return a <= 0 || b <= 0;
  };

  if (invalid_pair(m_ideal_heatmap_size.first, m_ideal_heatmap_size.second)) {
    throw std::invalid_argument(
        "HeatMap: ideal_heatmap_size must have positive dimensions");
  }

  if (invalid_pair(m_map_size.first, m_map_size.second)) {
    throw std::invalid_argument(
        "HeatMap: total_gridsize must have positive dimensions");
  }

  m_single_grid_size.first = m_map_size.first / m_ideal_heatmap_size.first;
  m_single_grid_size.second = m_map_size.second / m_ideal_heatmap_size.second;

  m_grid_values.resize(m_ideal_heatmap_size.first);
  for (std::vector<int> &x : m_grid_values) {
    x.resize(m_ideal_heatmap_size.second);
  }
}

std::vector<std::vector<int>>
cse498::HeatMap::OutPutHeatMap(std::ostream &output) {
  FillGrid();
  output << '\n';
  for (int r = 0; r < m_ideal_heatmap_size.first; ++r) {
    for (int c = 0; c < m_ideal_heatmap_size.second; ++c) {
      output << "----";
    }
    output << "-" << std::endl;
    for (int c = 0; c < m_ideal_heatmap_size.second; ++c) {
      output << "| " << m_grid_values[r][c] << " ";
    }
    output << "|" << std::endl;
  }

  for (int c = 0; c < m_ideal_heatmap_size.second; ++c) {
    output << "----";
  }
  output << "-" << std::endl;
  output << "Invalid Locations at: " << std::endl;
  for (std::pair<double, double> &x : m_invalid_locations) {
    output << x.first << "," << x.second << std::endl;
  }
  return m_grid_values;
}

std::vector<std::vector<int>> cse498::HeatMap::FillGrid() {
  auto in_bounds = [this](double x, double y, double grid_x, double grid_y) {
    return x < m_map_size.first && y < m_map_size.second &&
           grid_x >= 0 && grid_y >= 0;
  };

  for (std::vector<std::pair<double, double>>::iterator it =
           m_locations.begin();
       it != m_locations.end();) {
    double temp_grid_x = std::floor(it->first / m_single_grid_size.first);
    double temp_grid_y = std::floor(it->second / m_single_grid_size.second);
    // Check if in bounds
    if (in_bounds(it->first, it->second, temp_grid_x, temp_grid_y)) {
      m_grid_values[temp_grid_x][temp_grid_y] += 1;
    } else {
      m_invalid_locations.push_back(*it);
    }
    it = m_locations.erase(it);
  }
  return m_grid_values;
}

void cse498::HeatMap::LogLocations(ActionLog new_actions) {
  for (std::pair<double, double> &x : ParseActionLog(new_actions)) {
    m_locations.push_back(x);
  }
}

std::vector<std::pair<double, double>> cse498::HeatMap::GetInvalidInputs() {
  return m_invalid_locations;
}

std::vector<std::pair<double, double>>
cse498::HeatMap::ParseActionLog(ActionLog log) {
  std::vector<std::pair<double, double>> locations;
  for (auto &action : log.GetActions()) {
    if (action.ActionType == "move") {
      locations.push_back(
          std::pair<double, double>{action.Position.X(), action.Position.Y()});
    }
  }
  return locations;
}