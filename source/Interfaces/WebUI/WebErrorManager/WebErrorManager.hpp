/**
 * @file WebErrorManager.hpp
 * @brief Unified runtime error handling utility for the WebUI subsystem.
 *
 * This class provides a centralized interface for reporting runtime messages
 * across both native C++ and Emscripten (WebAssembly) environments.
 *
 * The WebErrorManager allows callers to:
 *      - Specify a severity level (Info, Warning, Error, Fatal)
 *      - Provide a message string
 *      - Control behavior via optional flags (alert, throw, exit)
 *
 * Under native builds:
 *      - Messages are printed to std::cout or std::cerr
 *
 * Under Emscripten builds:
 *      - Messages are routed to the browser console
 *      - Optional alert dialogs can be displayed for debugging
 *
 * Fatal errors can optionally:
 *      - Throw a std::runtime_error
 *      - Immediately terminate the program
 *
 * This utility replaces scattered console/alert calls and enables
 * consistent, extensible error handling across the project.
 *
 * Usage Example:
 *      WebErrorManager::Error("Something went wrong");
 *
 *      WebErrorManager::Fatal("Critical failure",
 *          {.throwOnFatal = true});
 *
 *      WebErrorManager::Warning("Check this state",
 *          {.showAlert = true});
 *
 * Additional Note:
 *      This implementation is intentionally lightweight and header-only.
 *      It is designed to be easily extended (e.g., GUI popups, logging systems,
 *      or integration with other modules).
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes and utilities correspond to the cse498 WebUI subsystem.
 */

#pragma once

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {

class WebErrorManager {
public:
    /**
     * @brief Severity levels for runtime messages.
     */
    enum class Severity {
        Info,
        Warning,
        Error,
        Fatal
    };

    /**
     * @brief Optional behavior flags for error reporting.
     */
    struct Options {
        bool showAlert = false;      ///< Show browser alert (Emscripten only)
        bool throwOnFatal = false;   ///< Throw exception on fatal error
        bool exitOnFatal = false;    ///< Exit program on fatal error

        Options(bool show = false, bool throwFatal = false, bool exitFatal = false)
        : showAlert(show), throwOnFatal(throwFatal), exitOnFatal(exitFatal) {}
    };

    ~WebErrorManager() = default;

    /**
     * @brief Primary reporting function.
     *
     * Routes message handling to the appropriate environment (native vs web).
     *
     * @param severity The severity level of the message
     * @param message The message content
     * @param options Optional behavior flags
     */
    static void Report(Severity severity,
                       const std::string& message,
                       const Options& options = Options{})
    {
#ifdef __EMSCRIPTEN__
        ReportEmscripten(severity, message, options);
#else
        ReportNative(severity, message, options);
#endif
    }

    /// Convenience wrapper for Info-level messages
    static void Info(const std::string& message,
                     const Options& options = Options{})
    {
        Report(Severity::Info, message, options);
    }

    /// Convenience wrapper for Warning-level messages
    static void Warning(const std::string& message,
                        const Options& options = Options{})
    {
        Report(Severity::Warning, message, options);
    }

    /// Convenience wrapper for Error-level messages
    static void Error(const std::string& message,
                      const Options& options = Options{})
    {
        Report(Severity::Error, message, options);
    }

    /// Convenience wrapper for Fatal-level messages
    static void Fatal(const std::string& message,
                      const Options& options = Options{})
    {
        Report(Severity::Fatal, message, options);
    }

private:
    /**
     * @brief Prefix string based on severity level.
     */
    static std::string Prefix(Severity severity)
    {
        switch (severity) {
            case Severity::Info:    return "[INFO] ";
            case Severity::Warning: return "[WARNING] ";
            case Severity::Error:   return "[ERROR] ";
            case Severity::Fatal:   return "[FATAL] ";
        }
        return "[UNKNOWN] ";
    }

#ifndef __EMSCRIPTEN__

    /**
     * @brief Native (non-web) error handling implementation.
     */
    static void ReportNative(Severity severity,
                             const std::string& message,
                             const Options& options)
    {
        const std::string formatted = Prefix(severity) + message;

        switch (severity) {
            case Severity::Info:
                std::cout << formatted << std::endl;
                break;

            case Severity::Warning:
            case Severity::Error:
            case Severity::Fatal:
                std::cerr << formatted << std::endl;
                break;
        }

        HandleFatal(severity, formatted, options);
    }

#else

    /**
     * @brief Emscripten/WebAssembly error handling implementation.
     *
     * Routes messages to the browser console and optionally displays alerts.
     */
    static void ReportEmscripten(Severity severity,
                                 const std::string& message,
                                 const Options& options)
    {
        const std::string formatted = Prefix(severity) + message;

        emscripten::val console = emscripten::val::global("console");

        switch (severity) {
            case Severity::Info:
                console.call<void>("log", formatted);
                break;

            case Severity::Warning:
                console.call<void>("warn", formatted);
                break;

            case Severity::Error:
            case Severity::Fatal:
                console.call<void>("error", formatted);
                break;
        }

        // Optional alert popup (useful for debugging UI issues)
        if (options.showAlert) {
            emscripten::val::global("alert")(formatted);
        }

        HandleFatal(severity, formatted, options);
    }

#endif

    /**
     * @brief Handles fatal error behavior (shared between environments).
     */
    static void HandleFatal(Severity severity,
                            const std::string& formatted,
                            const Options& options)
    {
        if (severity != Severity::Fatal) {
            return;
        }

        if (options.throwOnFatal) {
            throw std::runtime_error(formatted);
        }

        if (options.exitOnFatal) {
            std::exit(EXIT_FAILURE);
        }
    }
};

} // namespace cse498