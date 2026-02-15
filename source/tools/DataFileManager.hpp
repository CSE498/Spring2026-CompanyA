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

    // Helper function to convert various data types to a string representation for file storage. 
    std::string data_to_string(const T & data) {
        if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(data);
        }
        
        else if constexpr (std::is_same_v<T, std::vector<std::vector<char>>>) {
            std::string result;
            for (const auto & row : data) {
                for (const auto & cell : row) {
                    result += cell;
                }
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::vector<size_t>>) {
            std::string result;
            for (const auto & cell_type : data) {
              if (cell_type)
                result += std::to_string(cell_type);
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<char, size_t>>) {
          std::vector<std::pair<char, size_t>> items(data.begin(), data.end());

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
    // Constructor to initialize the DataFileManager with a filename and a pointer to the world. 
    DataFileManager(const std::string & filename, WorldBase *world)
    {
      m_filename = filename;
      m_world = world;
    }

    // Getter for the filename, allowing retrieval of the current filename being used by the DataFileManager. 
    std::string GetFilename() const { return m_filename; }

    template <typename T> 

    // Function to store data in a specific format, taking an ID, a type, and the data itself. 
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

    // Function to update the data file with the current state of the world, including tile and agent information. 
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