/**
 * @file DataFileManager.hpp
 * @brief A class to save the world data.
 * @note Status: PROPOSAL
 *
 * This link helped me to understand how to trigger functions to build a new row in the file:
 * https://stackoverflow.com/questions/67628186/how-to-write-the-result-of-a-function-to-a-file-c
 * 
 * The function data_to_string(const T & data) used this link to help me understand how to convert different data types to a string for storage in the file:
 * https://chatgpt.com/share/69925206-3bf4-8013-b807-859d6d7d1f89 
 * 
 * Learned and understand to check if two different types are convertible using std::is_convertible_v from this link: 
 * https://www.geeksforgeeks.org/cpp/stdis_convertible-template-in-c-with-examples/
 * 
 * Learned a modern way of how to make a string constant and view only using std::string_view from this link:
 * https://chatgpt.com/share/69a0eabd-00d8-8013-ac6b-83e572449254
 */


#pragma once

#include <cassert>
#include <string>
#include <fstream>
#include <iostream>
#include <string_view>

// For demonstration purposes, we are saving the data from MazeWorld
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/WorldGrid.hpp"
#include "../../source/Agents/PacingAgent.hpp"


namespace cse498 {
  constexpr int TILE_ID = 0;
  constexpr int AGENT_ID = 1;
  constexpr std::string_view TILE_TYPE = "Tile";
  constexpr std::string_view AGENT_TYPE = "Agent";
  constexpr std::string_view BACKGROUND_TYPE = "Background";

  // The DataFileManager class is responsible for managing the storage of world data into a file. 
  class DataFileManager
  {
  private:
    // The filename where the data will be stored. 
    // This is initialized in the constructor and can be retrieved using the GetFilename() method.
    std::string m_filename;

    //WorldBase &m_world; made it as a reference to guarantee that it is not null. 
    WorldBase &m_world; // This will be a unique_ptr stored in the world file once that file is complete

    template <typename T>

    // Helper function to convert various data types to a string representation for file storage. 
    std::string data_to_string(const T & data) const {
        if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(data);
        }
        
        else if constexpr (std::is_same_v<T, std::vector<std::vector<char>>>) {
            std::string result;
            for (const auto & row : data) {
                for (const auto & col : row) {
                    result += col;
                }
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::vector<size_t>>) {
            std::string result;
            for (const auto & cell_info : data) {
              if (cell_info)
                result += std::to_string(cell_info);
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<char, size_t>>) {
          std::vector<std::pair<char, size_t>> items(data.begin(), data.end());

          std::sort(items.begin(), items.end(), [](const auto & a, const auto & b) {
            return a.first < b.first; 
          });

          std::string result;
          for (const auto & item : items) {
            result += std::to_string(item.first);
            result += std::to_string(item.second);
          }
          return result;
        }
        else {
            throw std::runtime_error("cse498::DataFileManager::data_to_string(): Unconvertible data type");
        }
    }

    

  public:
    // Constructor to initialize the DataFileManager with a filename and a reference to the world. 
    DataFileManager(const std::string & filename, WorldBase *world) : 
    m_filename(filename), m_world(*world) 
    { 
        if (filename.empty()) {
            throw std::runtime_error("cse498::DataFileManager::Constructor: Filename cannot be empty");
        }
        if (world == nullptr) {
            throw std::runtime_error("cse498::DataFileManager::Constructor: World pointer cannot be null");
        }
    }

    // Getter for the filename, allowing retrieval of the current filename being used by the DataFileManager. 
    std::string GetFilename() const { return m_filename; }

    template <typename T> 

    // Function to store data in a specific format, taking an ID, a type, and the data itself. 
    std::string FormatData(int id, std::string_view type, const T & data) const {
        std::string data_stored = std::to_string(id);
        data_stored += "\t";
        
        if(type != TILE_TYPE && type != AGENT_TYPE && type != BACKGROUND_TYPE) {
            throw std::runtime_error("cse498::DataFileManager::FormatData(): Must provide a valid type [Tile, Agent, Background]");
        }
        data_stored += type;

        data_stored += "\t";

        std::string data_string = data_to_string(data);
        data_stored += data_string;

        return data_stored;
    }

    // Function to update the data file with the current state of the world, including tile and agent information. 
    bool Update()
    {
      std::ofstream file;
      file.open(m_filename, std::ofstream::app);
      if (!file.is_open())
      {
        std::cerr << "Unable to open file " << m_filename << std::endl;
        return false;
      }

      WorldGrid &grid = m_world.GetGrid();
      auto cells = grid.BuildSymbolMap();

      std::string tiles = FormatData(TILE_ID, TILE_TYPE, cells);
      PacingAgent agent(AGENT_ID, "Pacer", m_world);
      std::string agents = FormatData(AGENT_ID, AGENT_TYPE, m_world.GetKnownAgents(agent));

      file << tiles << "\n" << agents << "\n";

      file.close();

      return true;
    }

  protected:

  };

} // End of namespace cse498