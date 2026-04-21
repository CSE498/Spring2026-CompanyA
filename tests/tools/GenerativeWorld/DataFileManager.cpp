// #define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>

/*
/// @brief Test case for the constructor of the DataFileManager class,
///        ensuring that it initializes with the correct filename and world pointer.
TEST_CASE("Testing DataFileManager Constructor", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());

    CHECK(manager.GetFilename() == "DataFileManagerTest.json");
    std::remove("DataFileManagerTest.json");
}

/// @brief Test case for the Update function of the DataFileManager class,
///        ensuring that it successfully writes the current state of the world to the specified file.
TEST_CASE("Testing DataFileManager Update", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());

    manager.Update();

    /// @note Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.json");
    REQUIRE(file.is_open());

    std::string line;
    REQUIRE(std::getline(file, line)); ///< First line should be a JSON snapshot
    REQUIRE_FALSE(line.empty());
    REQUIRE(line.front() == '{');

    file.close();
    std::remove("DataFileManagerTest.json");
}

/// @brief Test case (error handling) for the Update function of the DataFileManager class,
///        ensuring that it returns false when it fails to open the specified file for writing.
TEST_CASE("Testing DataFileManager Update with invalid file", "[core]") {
    cse498::DataFileManager manager("/invalid_path/DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());

    manager.Update(); ///< Expected to silently fail (void, file cannot be opened)
    std::remove("DataFileManagerTest.json");
}

/// @brief Test case for the Update function of the DataFileManager class,
///        verifying that it successfully writes a JSON snapshot to the file.
TEST_CASE("Testing DataFileManager Update writing to file", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());

    manager.Update();

    /// @note Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.json");
    REQUIRE(file.is_open());

    std::string line;
    REQUIRE(std::getline(file, line)); ///< First line should be a JSON snapshot
    CHECK_FALSE(line.empty());
    CHECK(line.front() == '{');

    file.close();
    std::remove("DataFileManagerTest.json");
}

/// @brief Test case for the Update function of the DataFileManager class,
///        verifying that it successfully appends data to the file on multiple updates.
TEST_CASE("Testing DataFileManager Update with multiple updates", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());

    manager.Update();
    manager.Update(); ///< Update again to check appending

    /// @note Check that the file was created and has content
    std::ifstream file("DataFileManagerTest.json");
    REQUIRE(file.is_open());

    std::string line;
    int line_count = 0;
    while (std::getline(file, line)) {
        if (!line.empty()) ///< Only count non-empty lines
            line_count++;
    }
    CHECK(line_count >= 2); ///< At least 2 non-empty JSON lines expected after two updates

    file.close();
    std::remove("DataFileManagerTest.json");
}

/// @brief Test case for the Load function of the DataFileManager class,
///        verifying that it successfully reads the most recent tile snapshot back into the world grid.
TEST_CASE("Testing DataFileManager Load restores grid", "[core]") {
    cse498::DataFileManager manager("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());
    manager.Update();

    // Load back the snapshot into a fresh world
    cse498::DataFileManager loader("DataFileManagerTest.json", std::make_unique<cse498::MazeWorld>());
    loader.LoadData();

    std::remove("DataFileManagerTest.json");
}

/// @brief Test case (error handling) for the Load function of the DataFileManager class,
///        ensuring that it handles the case when the file does not exist.
TEST_CASE("Testing DataFileManager Load with missing file", "[core]") {
    cse498::DataFileManager manager("NonExistentFile.json", std::make_unique<cse498::MazeWorld>());

    std::stringstream buffer;
    auto old = std::cerr.rdbuf(buffer.rdbuf());
    manager.LoadData();
    std::cerr.rdbuf(old);
    REQUIRE(buffer.str().find("ERROR") != std::string::npos);
}
*/
