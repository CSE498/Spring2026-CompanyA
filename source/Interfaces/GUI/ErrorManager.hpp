/**
 * @file ErrorManager.hpp
 * @author Mark Sanchez
 * References made for Windows ANSI and function tips utilized Claude Sonnet 4.5
 */

#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace cse498 {

class ErrorManager {
private:
  // sharedptr to mGameUI -- will be used to throw sub-window errors once that
  // class is created

  std::string mFatalErrorColor = ErrorColor::BOLD_RED;
  std::string mTerminalErrorColor = ErrorColor::RED;
  std::string mTerminalWarningColor = ErrorColor::YELLOW;
  std::string mLogErrorColor = ErrorColor::CYAN;
  std::string mLogWarningColor = ErrorColor::MAGENTA;

  bool mColorEnabled = false;

  static bool SupportsColor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
      return false;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
      return false;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, dwMode);
#else
    return isatty(fileno(stderr));
#endif
  }

  void PrintInColor(const std::string &color,
                    const std::string &message) const {
    if (mColorEnabled) {
      std::cerr << color << message << ErrorColor::RESET << std::endl;
    } else {
      std::cerr << message << std::endl;
    }
  }

public:
  // ANSI color codes helper struct
  struct ErrorColor {
    static constexpr const char *RED = "\033[91m";
    static constexpr const char *YELLOW = "\033[93m";
    static constexpr const char *CYAN = "\033[96m";
    static constexpr const char *GREEN = "\033[92m";
    static constexpr const char *MAGENTA = "\033[95m";
    static constexpr const char *RESET = "\033[0m";
    static constexpr const char *BOLD = "\033[1m";
    static constexpr const char *BOLD_RED = "\033[1m\033[91m";
  };

  ErrorManager() : mColorEnabled(SupportsColor()) {}
  ~ErrorManager() = default;

  // --- Color setters ---
  void SetFatalErrorColor(const std::string &color) {
    mFatalErrorColor = color;
  }
  void SetTerminalErrorColor(const std::string &color) {
    mTerminalErrorColor = color;
  }
  void SetTerminalWarningColor(const std::string &color) {
    mTerminalWarningColor = color;
  }
  void SetLogErrorColor(const std::string &color) { mLogErrorColor = color; }
  void SetLogWarningColor(const std::string &color) {
    mLogWarningColor = color;
  }

  /// Prints message and immediately terminates the program.
  /// Use for unrecoverable states (e.g. failed memory allocation, corrupt game
  /// state).
  void RaiseFatalError(const std::string &message) const {
    PrintInColor(mFatalErrorColor, "[FATAL] " + message);
    std::exit(EXIT_FAILURE);
  }

  /// Prints an error message and continues execution.
  /// Use for recoverable issues the game logic handles elsewhere (e.g. invalid
  /// move, wall collision).
  void RaiseTerminalError(const std::string &message) const {
    PrintInColor(mTerminalErrorColor, "[ERROR] " + message);
  }

  /// Prints a warning message and continues execution.
  void RaiseTerminalWarning(const std::string &message) const {
    PrintInColor(mTerminalWarningColor, "[WARNING] " + message);
  }

  /// Use case - "You died", "Ran out of Time", "Not enough money", etc. TO DO
  static void GUIError(const std::string &message) {
    // TO DO with main game UI class, this line avoids "unused variable" in the
    // meantime
    (void)message;
  }

  /// Logs an error message and continues execution.
  void LogError(const std::string &message) const {
    PrintInColor(mLogErrorColor, "[LOG ERROR] " + message);
  }

  /// Logs a warning message and continues execution.
  void LogWarning(const std::string &message) const {
    PrintInColor(mLogWarningColor, "[LOG WARN] " + message);
  }
};

} // namespace cse498