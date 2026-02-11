/**
 * @file ErrorManager.hpp
 * @author Mark Sanchez
 * References made for Windows ANSI and function tips utilized Claude Sonnet 4.5
 */

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace cse498 {

// ANSI color codes
struct ErrorColor {
    static constexpr const char* RED     = "\033[91m";
    static constexpr const char* YELLOW  = "\033[93m";
    static constexpr const char* CYAN    = "\033[96m";
    static constexpr const char* GREEN   = "\033[92m";
    static constexpr const char* MAGENTA = "\033[95m";
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* BOLD    = "\033[1m";
};

class ErrorManager {
private:
    std::map<std::string, int> errors;

    std::string fatalErrorColor      = ErrorColor::BOLD;
    std::string terminalErrorColor   = ErrorColor::RED;
    std::string terminalWarningColor = ErrorColor::YELLOW;
    std::string logErrorColor        = ErrorColor::CYAN;
    std::string logWarningColor      = ErrorColor::MAGENTA;

    bool colorEnabled = false;

    bool SupportsColor() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return false;

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) return false;

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        return SetConsoleMode(hOut, dwMode);
#else
        return isatty(fileno(stderr));
#endif
    }

    void PrintInColor(const std::string& color, const std::string& message) const {
        if (colorEnabled) {
            std::cerr << color << message << ErrorColor::RESET << std::endl;
        } else {
            std::cerr << message << std::endl;
        }
    }

public:
    ErrorManager() : colorEnabled(SupportsColor()) {}
    ~ErrorManager() = default;

    // --- Color setters ---
    void SetFatalErrorColor(const std::string& color)      { fatalErrorColor = color; }
    void SetTerminalErrorColor(const std::string& color)   { terminalErrorColor = color; }
    void SetTerminalWarningColor(const std::string& color) { terminalWarningColor = color; }
    void SetLogErrorColor(const std::string& color)        { logErrorColor = color; }
    void SetLogWarningColor(const std::string& color)      { logWarningColor = color; }

    /// Prints message and immediately terminates the program.
    /// Use for unrecoverable states (e.g. failed memory allocation, corrupt game state).
    void RaiseFatalError(const std::string& message) {
        PrintInColor(fatalErrorColor, "[FATAL] " + message);
        std::exit(EXIT_FAILURE);
    }

    /// Prints an error message and continues execution.
    /// Use for recoverable issues the game logic handles elsewhere (e.g. invalid move, wall collision).
    void RaiseTerminalError(const std::string& message) {
        PrintInColor(terminalErrorColor, "[ERROR] " + message);
    }

    /// Prints a warning message and continues execution.
    void RaiseTerminalWarning(const std::string& message) {
        PrintInColor(terminalWarningColor, "[WARNING] " + message);
    }

    /// Use case - "You died", "Ran out of Time", "Not enough money", etc. TO DO
    void GUIError(const std::string& message);

    /// Logs an error message and continues execution.
    void LogError(const std::string& message) {
        PrintInColor(logErrorColor, "[LOG ERROR] " + message);
    }

    /// Logs a warning message and continues execution.
    void LogWarning(const std::string& message) {
        PrintInColor(logWarningColor, "[LOG WARN] " + message);
    }

    /// Clears errors
    void ClearErrors() { errors.clear(); }
};

} // namespace cse498