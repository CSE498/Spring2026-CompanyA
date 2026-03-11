/**
 * @file DataFileManager.hpp
 * @brief A class to save the world data.
 * @note Status: PROPOSAL
 *
 * This link helped me to understand how to trigger functions to build a new row in the file:
 * https://stackoverflow.com/questions/67628186/how-to-write-the-result-of-a-function-to-a-file-c
 * 
 * The function DataToString(const T & data) used this link to help me understand how to convert different data types to a string for storage in the file:
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

// The namespace cse498. 
namespace cse498 {
  // The DataFileManager class is responsible for managing the storage of world data into a file. 
  class DataFileManager {
  private:
    static constexpr std::string_view m_tile_type = "Tile";
    static constexpr std::string_view m_agent_type = "Agent";

    // The filename where the data will be stored. 
    // This is initialized in the constructor and can be retrieved using the GetFilename() method.
    std::string m_filename;

    std::unique_ptr<WorldBase> m_world;

    template <typename T>

    // Helper function to convert various data types to a string representation for file storage. 
    // This function takes any type of data that is convertible to a string. 
    std::string DataToString(const T & data) const {
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

          std::string result;
          for (const auto & item : items) {
            result += std::to_string(item.first);
            result += std::to_string(item.second);
          }
          return result;
        }
        throw std::runtime_error("cse498::DataFileManager::DataToString(): Unconvertible data type");
    }

    

    template <typename T> 

    // Helper function FormatData to store data in a specific format, taking a type and the data itself. 
    // This function takes any type of data as long as it is convertible to a string. 
    // This function formats the data by prefixing it with its type (such as "Tile" or "Agent") 
    // and separating the type and data with a tab character for easy parsing when reading from the file.
    std::string FormatData(std::string_view type, const T & data) const {
        if (type != m_tile_type && type != m_agent_type) 
          throw std::runtime_error("cse498::DataFileManager::FormatData(): Must provide a valid type [Tile, Agent]");

        std::string data_stored(type);
        data_stored += "\t";
        data_stored += DataToString(data);
        return data_stored;
    }

  public:
    // Constructor to initialize the DataFileManager with a filename and ownership of the world. 
    DataFileManager(const std::string & filename, std::unique_ptr<WorldBase> world) : 
    m_filename(filename), m_world(std::move(world)) 
    { 
        if (filename.empty()) 
          throw std::runtime_error("cse498::DataFileManager::Constructor: Filename cannot be empty");
        if (!m_world) 
          throw std::runtime_error("cse498::DataFileManager::Constructor: World pointer cannot be null");
    }

    // Getter for the filename, allowing retrieval of the current filename being used by the DataFileManager. 
    std::string GetFilename() const { return m_filename; }

    // Function to update the data file with the current state of the world, including tile and agent information. 
    void Update()
    {
      std::ofstream file;
      file.open(m_filename, std::ofstream::app);
      if (!file.is_open()){
        std::cerr << "cse498::DataFileManager::Update(): Failed to open file " << m_filename << std::endl;
        return;
      }

      // Capture the full grid layout as rows joined by ','
      std::ostringstream oss;
      m_world->GetGrid().Print(oss);
      std::string grid_str = oss.str();
      if (!grid_str.empty() && grid_str.back() == '\n') grid_str.pop_back();
      for (char & c : grid_str) if (c == '\n') c = ',';

      std::string tiles = FormatData(m_tile_type, grid_str);

      // Collect data for each agent: id,name,symbol,x,y — data blocks separated by '\t'
      std::string agent_data;
      for (size_t i = 0; i < m_world->GetNumAgents(); ++i) {
        const AgentBase & agent = m_world->GetAgent(i);
        if (i > 0) agent_data += "\t";
        agent_data += std::to_string(agent.GetID());
        agent_data += "," + agent.GetName();
        agent_data += "," + std::string(1, agent.GetSymbol());
        if (agent.GetLocation().IsPosition()) {
          const WorldPosition & pos = agent.GetLocation().AsWorldPosition();
          agent_data += "," + std::to_string(pos.X());
          agent_data += "," + std::to_string(pos.Y());
        } else {
          agent_data += ""; // no position available
        }
      }
      std::string agents = FormatData(m_agent_type, agent_data);

      file << tiles << "\n" << agents << "\n";

      file.close();
    }

    // Function to load (read) the most recent data from file into the world's grid. 
    void LoadData()
    {
      std::ifstream file(m_filename);
      if (!file.is_open()) {
        std::cerr << "cse498::DataFileManager::LoadData(): Failed to open file " << m_filename << std::endl;
        return;
      }

      std::string tile_data;
      std::string agent_data;
      std::string line;
      while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string type, data;
        if (!std::getline(iss, type, '\t')) continue;
        if (!std::getline(iss, data)) continue;
        if (type == m_tile_type)  tile_data  = data;
        if (type == m_agent_type) agent_data = data;
      }
      file.close();

      if (tile_data.empty()) {
        throw std::runtime_error("cse498::DataFileManager::LoadData(): No tile data found in file " + m_filename);
        return;
      }

      // Split the tile data by ',' to reconstruct the grid row-by-row
      std::vector<std::string> rows;
      std::istringstream row_stream(tile_data);
      std::string row;
      while (std::getline(row_stream, row, ',')) rows.push_back(row);

      if (rows.empty()) {
        throw std::runtime_error("cse498::DataFileManager::LoadData(): Failed to parse tile data from file " + m_filename);
        return;
      }
      m_world->GetGrid().Load(rows);

      // If no agent data available in the game world means there is no agent is present. 
      // So I just returned. Since zero agents is a valid state of the world, I don't throw an error there. 
      if (agent_data.empty()) return;

      // Restore agent positions: each data block is "id,name,symbol,x,y" separated by '\t'
      std::istringstream agent_stream(agent_data);
      std::string agent_data_block;
      while (std::getline(agent_stream, agent_data_block, '\t')) {
        // adb is the abbreviation for agent_data_block. 
        std::istringstream adb_stream(agent_data_block);
        std::string id_str, name, symbol, x_str, y_str;
        if (!std::getline(adb_stream, id_str, ',')) continue;
        if (!std::getline(adb_stream, name,   ',')) continue;
        if (!std::getline(adb_stream, symbol,',')) continue;
        if (!std::getline(adb_stream, x_str,  ',')) continue;
        if (!std::getline(adb_stream, y_str,  ',')) continue;
        if (x_str.empty() || y_str.empty()) continue;

        size_t agent_id = std::stoul(id_str);
        double x = std::stod(x_str);
        double y = std::stod(y_str);

        // Find the matching agent in the world and restore its position
        if (agent_id < m_world->GetNumAgents()) {
          AgentBase & agent = m_world->GetAgent(agent_id);
          agent.SetLocation(Location(WorldPosition(x, y)));
          if (!symbol.empty()) agent.SetSymbol(symbol[0]);
        }
      }
    }
  };
}; // End of namespace cse498