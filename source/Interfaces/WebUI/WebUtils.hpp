#pragma once

#include <emscripten/val.h>
#include <print>
#include <string>

using emscripten::val, std::string;

namespace cse498 {

/// @brief Returns the browser document object.
/// @return a reference to the document object
inline val GetDocument() { return val::global("document"); }

/// @brief Returns the browser console object.
/// @return a reference to the console object
inline val GetConsole() { return val::global("console"); }

/// @brief Returns the element associated with this Id.
/// @param Id the Id of the element
/// @return a reference to the html element or val::undefined() if not found
inline val GetElement(const string& Id) { return GetDocument().call<val>("getElementById", Id); }

/// @brief Returns the specified property of the element.
/// @param element the element to check
/// @param property the property to get
/// @return a reference to the element's property or val::undefined() if not found
inline val GetProperty(val element, const string& property) {
    if (element.isNull() || element.isUndefined())
        return val::undefined();

    return element[property];
}

/// @brief Returns the value of the specified css property for this element.
///        It checks the css string to avoid unpredictable browser optimizations
///        causing the values to be equivalent, but different than what we set.
/// @param element the element to check
/// @param property the css property to get the value of
/// @return a string representing the value of the property, empty if not found
inline string GetCSSProperty(val element, const string& property) {
    if (element.isNull() || element.isUndefined())
        return "";

    val style_attr = element.call<val>("getAttribute", string("style"));
    if (!style_attr.isString())
        return "";

    string style_str = style_attr.as<string>();

    // Look for " property:" (with a space) or ";property:" or at the very start
    // A more robust way is to check the character before the match.
    string search_key = property + ":";
    size_t start = 0;

    while ((start = style_str.find(search_key, start)) != string::npos) {
        // Check if it's a true match
        bool is_at_start = (start == 0);
        bool after_semicolon = (!is_at_start && style_str[start - 1] == ';');
        bool after_space = (!is_at_start && style_str[start - 1] == ' ');

        // If it's preceded by a hyphen (like in 'border-width'), it's a false match.
        // We want to skip it and keep searching.
        if (!is_at_start && style_str[start - 1] == '-') {
            start += search_key.length();
            continue;
        }

        // If we reach here, we've found the actual property
        start += search_key.length();
        size_t end = style_str.find(";", start);

        string value = (end == string::npos) ? style_str.substr(start) : style_str.substr(start, end - start);

        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        return value;
    }

    return "";
}

} // namespace cse498
