/**
 * @file DataFileManager.hpp
 * @brief A class to save the world data.
 * @note Status: PROPOSAL
 *
 * @details
 * Reference material consulted while drafting this proposal:
 * This link helped me to understand how to trigger functions to build a new row in the file:
 * https://stackoverflow.com/questions/67628186/how-to-write-the-result-of-a-function-to-a-file-c
 * The function DataToString(const T & data) used this link to help me understand how to convert different data types to a string for storage in the file:
 * https://chatgpt.com/share/69925206-3bf4-8013-b807-859d6d7d1f89 
 * Learned and understand to check if two different types are convertible using std::is_convertible_v from this link: 
 * https://www.geeksforgeeks.org/cpp/stdis_convertible-template-in-c-with-examples/
 * Learned a modern way of how to make a string constant and view only using std::string_view from this link:
 * https://chatgpt.com/share/69a0eabd-00d8-8013-ac6b-83e572449254
 * Understood and learned about json from this link: https://www.w3schools.com/whatis/whatis_json.asp
 */


#pragma once

#include <cassert>
#include <cctype>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>
#include <vector>

/// @brief Includes used for demonstration with MazeWorld data.
#include "../../source/Worlds/MazeWorld.hpp"
#include "../../source/core/WorldBase.hpp"
#include "../../source/core/WorldGrid.hpp"
#include "../../source/Agents/PacingAgent.hpp"

/// @brief Main namespace for the CSE 498 project code.
namespace cse498 {
    /**
     * @brief Manages saving and loading world state snapshots.
     */
    class DataFileManager {
    private:
        /// @brief File path where JSON snapshots are stored.
        std::string m_filename;

        /// @brief Owned world instance whose state is serialized and restored.
        std::unique_ptr<WorldBase> m_world;

        /**
         * @brief Escapes a string for safe inclusion in JSON output.
         * @param input The raw input string.
         * @return The escaped JSON-safe string.
         */
        static std::string EscapeJsonString(const std::string &input) {
            std::string escaped;
            escaped.reserve(input.size());

            for (char ch: input) {
                switch (ch) {
                    case '\\': escaped += "\\\\";
                        break;
                    case '"': escaped += "\\\"";
                        break;
                    case '\n': escaped += "\\n";
                        break;
                    case '\r': escaped += "\\r";
                        break;
                    case '\t': escaped += "\\t";
                        break;
                    default: escaped += ch;
                        break;
                }
            }

            return escaped;
        }

        /**
         * @brief Advances past whitespace while parsing JSON text.
         * @param text The source text being parsed.
         * @param pos The current parse position, updated in place.
         */
        static void SkipWhitespace(const std::string &text, size_t &pos) {
            while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
                ++pos;
            }
        }

        /**
         * @brief Consumes an expected character after skipping whitespace.
         * @param text The source text being parsed.
         * @param pos The current parse position, updated in place.
         * @param expected The character that must appear next.
         * @return True if the expected character was consumed.
         */
        static bool ExpectChar(const std::string &text, size_t &pos, char expected) {
            SkipWhitespace(text, pos);
            if (pos >= text.size() || text[pos] != expected) return false;
            ++pos;
            return true;
        }

        /**
         * @brief Parses a quoted JSON string, including supported escape sequences.
         * @param text The source text being parsed.
         * @param pos The current parse position, updated in place.
         * @param out Receives the parsed string on success.
         * @return True if a valid quoted string was parsed.
         */
        static bool ParseQuotedString(const std::string &text, size_t &pos, std::string &out) {
            SkipWhitespace(text, pos);
            if (pos >= text.size() || text[pos] != '"') return false;
            ++pos;

            std::string value;
            while (pos < text.size()) {
                char ch = text[pos++];
                if (ch == '"') {
                    out = value;
                    return true;
                }

                if (ch == '\\') {
                    if (pos >= text.size()) return false;
                    const char escaped = text[pos++];
                    switch (escaped) {
                        case '"': value += '"';
                            break;
                        case '\\': value += '\\';
                            break;
                        case 'n': value += '\n';
                            break;
                        case 'r': value += '\r';
                            break;
                        case 't': value += '\t';
                            break;
                        default: value += escaped;
                            break;
                    }
                    continue;
                }

                value += ch;
            }

            return false;
        }

        /**
         * @brief Parses an integer or floating-point token from JSON text.
         * @param text The source text being parsed.
         * @param pos The current parse position, updated in place.
         * @param token Receives the parsed numeric token.
         * @return True if a valid number token was parsed.
         */
        static bool ParseNumberToken(const std::string &text, size_t &pos, std::string &token) {
            SkipWhitespace(text, pos);
            const size_t start = pos;

            if (pos < text.size() && (text[pos] == '-' || text[pos] == '+')) ++pos;
            while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) ++pos;
            if (pos < text.size() && text[pos] == '.') {
                ++pos;
                while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) ++pos;
            }
            if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E')) {
                ++pos;
                if (pos < text.size() && (text[pos] == '-' || text[pos] == '+')) ++pos;
                while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) ++pos;
            }

            if (pos == start) return false;
            token = text.substr(start, pos - start);
            return true;
        }

        /**
         * @brief Parses a fixed JSON literal such as true, false, or null.
         * @param text The source text being parsed.
         * @param pos The current parse position, updated in place.
         * @param literal The exact literal expected at the current position.
         * @return True if the literal was found and consumed.
         */
        static bool ParseLiteral(const std::string &text, size_t &pos, std::string_view literal) {
            SkipWhitespace(text, pos);
            if (text.compare(pos, literal.size(), literal) != 0) return false;
            pos += literal.size();
            return true;
        }

    public:
        /**
         * @brief Constructs a data manager for a specific world snapshot file.
         * @param filename Path to the snapshot file.
         * @param world Owned world instance to manage.
         * @throws std::runtime_error If the filename is empty or the world pointer is null.
         */
        DataFileManager(const std::string &filename, std::unique_ptr<WorldBase> world) : m_filename(filename),
            m_world(std::move(world)) {
            if (filename.empty())
                throw std::runtime_error("cse498::DataFileManager::Constructor: Filename cannot be empty");
            if (!m_world)
                throw std::runtime_error("cse498::DataFileManager::Constructor: World pointer cannot be null");
        }

        /**
         * @brief Returns the snapshot filename.
         * @return The configured filename.
         */
        std::string GetFilename() const { return m_filename; }

        /**
         * @brief Returns mutable access to the managed world.
         * @return A reference to the managed world.
         */
        WorldBase &GetWorld() { return *m_world; }

        /**
         * @brief Returns read-only access to the managed world.
         * @return A const reference to the managed world.
         */
        const WorldBase &GetWorldView() const { return *m_world; }

        /**
         * @brief Appends the current world state to the snapshot file as JSON.
         */
        void Update() {
            std::ofstream file;
            file.open(m_filename, std::ofstream::app);
            if (!file.is_open()) {
                std::cerr << "ERROR::cse498::DataFileManager::Update(): Failed to open file " << m_filename <<
                        std::endl;
                return;
            }

            const auto GridToTextRows = [this]() {
                std::ostringstream oss;
                m_world->GetGrid().Print(oss);

                std::string grid_str = oss.str();
                if (!grid_str.empty() && grid_str.back() == '\n') grid_str.pop_back();

                std::vector<std::string> txt_rows;
                std::istringstream row_stream(grid_str);
                std::string txt_row;
                while (std::getline(row_stream, txt_row)) {
                    txt_rows.push_back(txt_row);
                }

                return txt_rows;
            };

            const std::vector<std::string> tiles = GridToTextRows();

            std::ostringstream json;
            json << "{\"tiles\":[";
            for (size_t i = 0; i < tiles.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << EscapeJsonString(tiles[i]) << "\"";
            }
            json << "],\"agents\":[";

            for (size_t i = 0; i < m_world->GetNumAgents(); ++i) {
                const AgentBase &agent = m_world->GetAgent(i);
                if (i > 0) json << ",";

                json << "{\"id\":" << agent.GetID();
                json << ",\"name\":\"" << EscapeJsonString(agent.GetName()) << "\"";
                json << ",\"symbol\":\"" << EscapeJsonString(std::string(1, agent.GetSymbol())) << "\"";

                if (agent.GetLocation().IsPosition()) {
                    const WorldPosition &pos = agent.GetLocation().AsWorldPosition();
                    json << ",\"x\":" << pos.X() << ",\"y\":" << pos.Y();
                } else {
                    json << ",\"x\":null,\"y\":null";
                }

                json << "}";
            }
            json << "]}";

            file << json.str() << "\n";

            file.close();
        }

        /**
         * @brief Loads the most recent world snapshot from the file.
         * @throws std::runtime_error If the file contains no valid snapshot or malformed tile data.
         */
        void LoadData() {
            std::ifstream file(m_filename);
            if (!file.is_open()) {
                std::cerr << "ERROR::cse498::DataFileManager::LoadData(): Failed to open file " << m_filename <<
                        std::endl;
                return;
            }

            std::string last_valid_line;
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty()) last_valid_line = line;
            }
            file.close();

            if (last_valid_line.empty()) {
                throw std::runtime_error(
                    "ERROR::cse498::DataFileManager::LoadData(): No JSON snapshot found in file " + m_filename);
            }

            /**
             * @brief Parse the tiles array from a snapshot such as:
             * {"tiles":["...","..."],"agents":[{...}]}
             */
            std::vector<std::string> rows; {
                size_t pos = last_valid_line.find("\"tiles\"");
                if (pos == std::string::npos) {
                    throw std::runtime_error("cse498::DataFileManager::LoadData(): Missing 'tiles' key in JSON data");
                }
                pos = last_valid_line.find('[', pos);
                if (pos == std::string::npos) {
                    throw std::runtime_error("cse498::DataFileManager::LoadData(): Invalid 'tiles' array in JSON data");
                }
                ++pos;

                while (true) {
                    SkipWhitespace(last_valid_line, pos);
                    if (pos >= last_valid_line.size()) {
                        throw std::runtime_error("cse498::DataFileManager::LoadData(): Unterminated 'tiles' array");
                    }
                    if (last_valid_line[pos] == ']') {
                        ++pos;
                        break;
                    }

                    std::string row;
                    if (!ParseQuotedString(last_valid_line, pos, row)) {
                        throw std::runtime_error(
                            "cse498::DataFileManager::LoadData(): Invalid row string in 'tiles' array");
                    }
                    rows.push_back(row);

                    SkipWhitespace(last_valid_line, pos);
                    if (pos < last_valid_line.size() && last_valid_line[pos] == ',') {
                        ++pos;
                    }
                }
            }

            if (rows.empty()) {
                throw std::runtime_error(
                    "cse498::DataFileManager::LoadData(): Failed to parse tile data from file " + m_filename);
            }
            m_world->GetGrid().Load(rows);

            /// @brief Restore agent state from the optional agents array.
            size_t pos = last_valid_line.find("\"agents\"");
            if (pos == std::string::npos) return;
            pos = last_valid_line.find('[', pos);
            if (pos == std::string::npos) return;
            ++pos;

            while (true) {
                SkipWhitespace(last_valid_line, pos);
                if (pos >= last_valid_line.size()) break;
                if (last_valid_line[pos] == ']') {
                    ++pos;
                    break;
                }
                if (!ExpectChar(last_valid_line, pos, '{')) break;

                size_t agent_id = 0;
                bool has_id = false;
                std::string symbol;
                bool has_x = false;
                bool has_y = false;
                double x = 0.0;
                double y = 0.0;

                while (true) {
                    std::string key;
                    if (!ParseQuotedString(last_valid_line, pos, key)) break;
                    if (!ExpectChar(last_valid_line, pos, ':')) break;

                    if (key == "id") {
                        std::string id_token;
                        if (!ParseNumberToken(last_valid_line, pos, id_token)) break;
                        agent_id = std::stoul(id_token);
                        has_id = true;
                    } else if (key == "symbol") {
                        if (!ParseQuotedString(last_valid_line, pos, symbol)) break;
                    } else if (key == "x") {
                        if (ParseLiteral(last_valid_line, pos, "null")) {
                            has_x = false;
                        } else {
                            std::string x_token;
                            if (!ParseNumberToken(last_valid_line, pos, x_token)) break;
                            x = std::stod(x_token);
                            has_x = true;
                        }
                    } else if (key == "y") {
                        if (ParseLiteral(last_valid_line, pos, "null")) {
                            has_y = false;
                        } else {
                            std::string y_token;
                            if (!ParseNumberToken(last_valid_line, pos, y_token)) break;
                            y = std::stod(y_token);
                            has_y = true;
                        }
                    } else {
                        std::string ignored;
                        if (!ParseQuotedString(last_valid_line, pos, ignored)) {
                            std::string ignore_val;
                            ParseNumberToken(last_valid_line, pos, ignore_val);
                        }
                    }

                    SkipWhitespace(last_valid_line, pos);
                    if (pos < last_valid_line.size() && last_valid_line[pos] == ',') {
                        ++pos;
                        continue;
                    }
                    if (pos < last_valid_line.size() && last_valid_line[pos] == '}') {
                        ++pos;
                        break;
                    }
                }

                if (has_id && agent_id < m_world->GetNumAgents()) {
                    AgentBase &agent = m_world->GetAgent(agent_id);
                    if (!symbol.empty()) agent.SetSymbol(symbol[0]);
                    if (has_x && has_y) {
                        agent.SetLocation(Location(WorldPosition(x, y)));
                    }
                }

                SkipWhitespace(last_valid_line, pos);
                if (pos < last_valid_line.size() && last_valid_line[pos] == ',') {
                    ++pos;
                    continue;
                }
            }
        }
    };
}; ///< namespace cse498
