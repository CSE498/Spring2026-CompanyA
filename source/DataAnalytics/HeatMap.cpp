#include "HeatMap.hpp"

#include <cmath>
#include <iostream>
#include <print>
#include <utility>
#include <vector>
// #include "ActionLog.hpp"

// Constexpr this part
cse498::HeatMap::HeatMap(ActionLog log, std::pair<int, int> ideal_gridsize,
                         std::pair<double, double> total_gridsize) {
  m_locations = ParseActionLog(log);
  m_ideal_gridsize = ideal_gridsize;
  m_total_gridsize = total_gridsize;

  auto invalid_pair = [](auto a, auto b) {
      return a <= 0 || b <= 0;
  };
  
  if (invalid_pair(m_ideal_gridsize.first, m_ideal_gridsize.second)) {
    std::cout << "HeatMap ideal gridsize negative assign new values for valid "
                 "heatmap";
    m_ideal_gridsize.first = 10;
    m_ideal_gridsize.second = 10;
  }

  if (invalid_pair(m_total_gridsize.first, m_total_gridsize.second)) {
    std::cout << "HeatMap given negative total gridsize values assign new "
                 "values for valid heatmap";
    m_total_gridsize.first = 1000;
    m_total_gridsize.second = 1000;
  }
  if ((m_ideal_gridsize.first / m_total_gridsize.first) > .65 &&
      m_ideal_gridsize.second / m_total_gridsize.second > .65) {
    std::cout << "Warning Heatmap ideal gridsize is to close or larger in size "
                 "than total gridsize heatmap may be hard to visualize";
  }

  m_single_grid_size.first = total_gridsize.first / ideal_gridsize.first;
  m_single_grid_size.second = total_gridsize.second / ideal_gridsize.second;

  m_grid_values.resize(m_ideal_gridsize.first);
  for (std::vector<int> &x : m_grid_values) {
    x.resize(m_ideal_gridsize.second);
  }
}

std::vector<std::vector<int>>
cse498::HeatMap::OutPutHeatMap(std::ostream &output) {
  FillGrid();
  output << '\n';
  for (int r = 0; r < m_ideal_gridsize.first; ++r) {
    for (int c = 0; c < m_ideal_gridsize.second; ++c) {
      // std::print("----");
      output << "----";
    }
    // std::println("-");
    output << "-" << std::endl;
    for (int c = 0; c < m_ideal_gridsize.second; ++c) {
      // std::print("| {} ", m_grid_values[r][c]);
      output << "| " << m_grid_values[r][c] << " ";
    }
    // std::println("|");
    output << "|" << std::endl;
  }

  for (int c = 0; c < m_ideal_gridsize.second; ++c) {
    // std::print("----");
    output << "----";
  }
  // std::println("-");
  output << "-" << std::endl;
  // std::println("Invalid Locations at: ");
  output << "Invalid Locations at: " << std::endl;
  for (std::pair<double, double> &x : m_invalid_locations) {
    // std::println("{},{}", x.first, x.second);
    output << x.first << "," << x.second << std::endl;
  }
  return m_grid_values;
}

void cse498::HeatMap::FillGrid() {
  auto in_bounds = [this](double x, double y, double grid_x, double grid_y) {
  return x < m_total_gridsize.first &&
          y < m_total_gridsize.second &&
          grid_x >= 0 &&
          grid_y >= 0;
  };

  for (std::vector<std::pair<double, double>>::iterator it =
           m_locations.begin();
       it != m_locations.end();) {
    double temp_grid_x = std::floor(it->first / m_single_grid_size.first);
    double temp_grid_y = std::floor(it->second / m_single_grid_size.second);
    // Check if in bounds
    if (in_bounds(it->first, it->second, temp_grid_x, temp_grid_y)){
      m_grid_values[temp_grid_x][temp_grid_y] += 1;
    } else {
      m_invalid_locations.push_back(*it);
    }
    m_locations.erase(it);
  }
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