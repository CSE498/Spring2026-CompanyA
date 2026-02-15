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

  public:
    DataFileManager(const std::string & filename, WorldBase *world)
    {
      m_filename = filename;
      m_world = world;
    }

    std::string GetFilename() const { return m_filename; }

    template <typename T> std::string StoreData(int id, std::string type, T data) {
        std::string data_stored = std::to_string(id);
        data_stored += "\t";
        
        if(type == "Tile" || type == "Agent" || type == "Background") {
        }
        else {
            throw std::runtime_error("cse498::DataFileManager::StoreData(): Must provide a vaild type [Tile, Agent, Background]");
        }
        data_stored += type;

        data_stored += "\t";

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