#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/tools/DataFileManager.hpp"
#include "../../source/core/WorldBase.hpp"




TEST_CASE("Testing DataFileManager Constructor", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.GetFilename() == "DataFileManagerTest.csv");
}

TEST_CASE("Testing DataFileManager StoreData", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.StoreData(3, "Tile", "Temporary Data") == "3\tTile\tTemporary Data");
}

TEST_CASE("Testing DataFileManager Update", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    manager.Update();

    // Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.csv");
    REQUIRE(file.is_open());

    std::string line;
    REQUIRE(std::getline(file, line)); // First line should be tile data
    REQUIRE_FALSE(line.empty());

    REQUIRE(std::getline(file, line)); // Second line should be agent data
    REQUIRE_FALSE(line.empty());

    file.close();
}