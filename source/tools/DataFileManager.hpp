/**
 * @file DataFileManager.hpp
 * @brief A class to save the world data.
 * @note Status: PROPOSAL
 *
 * This link helped me to understand how to trigger functions to build a new row in the file:
 * https://stackoverflow.com/questions/67628186/how-to-write-the-result-of-a-function-to-a-file-c
 */


#pragma once

#include <cassert>
#include <string>
#include <fstream>
#include <iostream>

// For demonstration purposes, we are saving the data from MazeWorld
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/WorldGrid.hpp"
#include "../../source/Agents/PacingAgent.hpp"


namespace cse498 {

  class DataFileManager
  {
  private:
    std::string m_filename;

    //MazeWorld *m_world; // This will be a unique_ptr stored in the world file once that file is complete
    WorldBase *m_world; // This will be a unique_ptr stored in the world file once that file is complete

    template <typename T>
    std::string data_to_string(const T & data) {
        if constexpr (std::is_convertible_v<T, std::string_view>) {
            return std::string(data);
        }

        else if constexpr (std::is_same_v<T, const char*>) {
          return std::string(data);
        }
        
        else if constexpr (std::is_same_v<T, std::vector<std::vector<char>>>) {
            std::string result;
            for (const auto & row : data) {
                for (const auto & cell : row) {
                    result += cell;
                }
                result += "\n"; // Newline after each row
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::vector<size_t>>) {
            std::string result;
            for (const auto & cell_type : data) {
              if (cell_type)
                result += std::to_string(cell_type) + "\n";
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<char, size_t>>) {
          std::vector<std::pair<char, size_t>> items(data.begin(), data.end());
          std::sort(items.begin(), items.end(),
                    [](auto &a, auto &b){ return a.first < b.first; });

          std::string result;
          for (const auto & item : items) {
            result += item.first;
            result += ":";
            result += std::to_string(item.second);
            result += "\n";
          }
          return result;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::tuple<size_t, size_t, char>>>) {
            std::string result;
            for (const auto & info : data) {
                size_t id, x, y;
                char symbol;
                std::tie(id, x, y, symbol) = info;
                result += "ID: " + std::to_string(id) + ", X: " + std::to_string(x) + ", Y: " + std::to_string(y) + ", Symbol: " + symbol + "\n";
            }
            return result;
        }
        else {
            throw std::runtime_error("cse498::DataFileManager::data_to_string(): Unsupported data type");
        }
    }



  public:
    DataFileManager(const std::string & filename, WorldBase *world)
    {
      m_filename = filename;
      m_world = world;
    }

    std::string GetFilename() const { return m_filename; }

    template <typename T> 
    std::string StoreData(int id, std::string type, const T & data) {
        std::string data_stored = std::to_string(id);
        data_stored += "\t";
        
        if(type == "Tile" || type == "Agent" || type == "Background") {
        }
        else {
            throw std::runtime_error("cse498::DataFileManager::StoreData(): Must provide a vaild type [Tile, Agent, Background]");
        }
        data_stored += type;

        data_stored += "\t";

        std::string data_string = data_to_string(data);
        data_stored += data_string;

        return data_stored;
    }

    void Update()
    {
      std::ofstream file;
      file.open(m_filename, std::ofstream::app);
      if (!file.is_open())
      {
        std::cerr << "Unable to open file " << m_filename << std::endl;
        return;
      }

      WorldGrid &grid = m_world->GetGrid();
      auto cells = grid.BuildSymbolMap();

      std::string tiles = StoreData(0, "Tile", cells);
      PacingAgent agent(0, "Pacer", *m_world);
      std::string agents = StoreData(1, "Agent", m_world->GetKnownAgents(agent));

      file << tiles << "\n" << agents << "\n";
      file.close();
    }

  protected:

  };

} // End of namespace cse498