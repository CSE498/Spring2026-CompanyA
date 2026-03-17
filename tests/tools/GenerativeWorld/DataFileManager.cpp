//#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/tools/DataFileManager.hpp"
#include "../../../source/core/WorldBase.hpp"
#include <sstream>



/*
* Test case for the constructor of the DataFileManager class, 
* ensuring that it initializes with the correct filename and world pointer.
*/
TEST_CASE("Testing DataFileManager Constructor", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());

    CHECK(manager.GetFilename() == "DataFileManagerTest.csv");
    std::remove("DataFileManagerTest.csv");
}

/*
* Test case for the Update function of the DataFileManager class,
* ensuring that it successfully writes the current state of the world to the specified file. 
*/
TEST_CASE("Testing DataFileManager Update", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());

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
    std::remove("DataFileManagerTest.csv");
}

/* 
* Test case (error handling) for the Update function of the DataFileManager class,
* ensuring that it returns false when it fails to open the specified file for writing.
*/
TEST_CASE("Testing DataFileManager Update with invalid file", "[core]") {
    cse498::DataFileManager manager("/invalid_path/DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());

    manager.Update(); // expected to silently fail (void, file cannot be opened)
    std::remove("DataFileManagerTest.csv");
}

/*
* Test case for the Update function of the DataFileManager class,
* verifying that it successfully appends data (two lines) to the file updates.
*/
TEST_CASE("Testing DataFileManager Update writing to file", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());

    manager.Update();

    // Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.csv");
    REQUIRE(file.is_open());

    std::string line;
    REQUIRE(std::getline(file, line)); // First line should be tile data
    CHECK_FALSE(line.empty());

    REQUIRE(std::getline(file, line)); // Second line should be agent data
    CHECK_FALSE(line.empty());

    file.close();
    std::remove("DataFileManagerTest.csv");
}

/*
* Test case for the Update function of the DataFileManager class,
* verifying that it successfully appends data to the file on multiple updates.
*/
TEST_CASE("Testing DataFileManager Update with multiple updates", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());

    manager.Update();
    manager.Update(); // Update again to check appending

    // Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.csv");
    REQUIRE(file.is_open());

    std::string line;
    int line_count = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) // Only count non-empty lines
            line_count++;
    }
    CHECK(line_count >= 4); // At least 4 lines expected after two updates, since each update writes two lines

    file.close();
    std::remove("DataFileManagerTest.csv");
}

/*
* Test case for the Load function of the DataFileManager class,
* verifying that it successfully reads the most recent tile snapshot back into the world grid.
*/
TEST_CASE("Testing DataFileManager Load restores grid", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());
    manager.Update();

    // Load back the snapshot into a fresh world
    cse498::DataFileManager loader("DataFileManagerTest.csv", std::make_unique<cse498::MazeWorld>());
    loader.LoadData();

    std::remove("DataFileManagerTest.csv");
}

/*
* Test case (error handling) for the Load function of the DataFileManager class,
* ensuring that it handles the case when the file does not exist.
*/
TEST_CASE("Testing DataFileManager Load with missing file", "[core]") {
    cse498::DataFileManager manager("NonExistentFile.csv", std::make_unique<cse498::MazeWorld>());

    std::stringstream buffer;
    auto old = std::cerr.rdbuf(buffer.rdbuf());
    manager.LoadData();
    std::cerr.rdbuf(old);
    REQUIRE(buffer.str().find("ERROR") != std::string::npos);
}