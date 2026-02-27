#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../../source/tools/DataFileManager.hpp"
#include "../../../source/core/WorldBase.hpp"



/*
* Test case for the constructor of the DataFileManager class, 
* ensuring that it initializes with the correct filename and world pointer.
*/
TEST_CASE("Testing DataFileManager Constructor", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.GetFilename() == "DataFileManagerTest.csv");
}

/*
* Test case for the FormatData function of the DataFileManager class,
* verifying that it correctly formats the data string for storage. 
*/
TEST_CASE("Testing DataFileManager FormatData", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.FormatData(3, "Tile", "Temporary Data") == "3\tTile\tTemporary Data");
}


/*
* Test case for the Update function of the DataFileManager class,
* ensuring that it successfully writes the current state of the world to the specified file. 
*/
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

/*
* Test case (error handling) for the FormatData function of the DataFileManager class,
* verifying that it throws a runtime error when an invalid type is provided.
*/
TEST_CASE("Testing DataFileManager FormatData with invalid type", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK_THROWS_AS(manager.FormatData(3, "InvalidType", "Temporary Data"), std::runtime_error);
}

/*
* Test case (error handling) for the FormatData function of the DataFileManager class,
* ensuring that it throws a runtime error when an unconvertible data type is provided.
*/
TEST_CASE("Testing DataFileManager FormatData with unconvertible data type", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    struct UnconvertibleType {};
    UnconvertibleType data;

    CHECK_THROWS_AS(manager.FormatData(3, "Tile", data), std::runtime_error);
}

/*
* Test case (error handling) for the FormatData function of the DataFileManager class,
* verifying that it throws a runtime error when an empty type string is provided.
*/
TEST_CASE("Testing DataFileManager FormatData when type is empty", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK_THROWS_AS(manager.FormatData(3, "", "Temporary Data"), std::runtime_error);
}


/* 
* Test case (error handling) for the Update function of the DataFileManager class,
* ensuring that it returns false when it fails to open the specified file for writing.
*/
TEST_CASE("Testing DataFileManager Update with invalid file", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("/invalid_path/DataFileManagerTest.csv", world.get());

    CHECK_FALSE(manager.Update());
}

/*
* Test case for the Update function of the DataFileManager class,
* verifying that it successfully appends data (two lines) to the file updates.
*/
TEST_CASE("Testing DataFileManager Update writing to file", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.Update());

    // Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.csv");
    REQUIRE(file.is_open());

    std::string line;
    REQUIRE(std::getline(file, line)); // First line should be tile data
    CHECK_FALSE(line.empty());

    REQUIRE(std::getline(file, line)); // Second line should be agent data
    CHECK_FALSE(line.empty());

    file.close();
}

/*
* Test case for the Update function of the DataFileManager class,
* verifying that it successfully appends data to the file on multiple updates.
*/
TEST_CASE("Testing DataFileManager Update with multiple updates", "[core]") {
    std::unique_ptr world = std::make_unique<cse498::MazeWorld>();
    cse498::DataFileManager manager("DataFileManagerTest.csv", world.get());

    CHECK(manager.Update());
    CHECK(manager.Update()); // Update again to check appending

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
}