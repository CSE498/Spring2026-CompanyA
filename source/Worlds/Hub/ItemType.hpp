/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Shared item and resource types for interactive world systems.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string_view>

namespace cse498 {

/**
 * @enum ItemType
 * @brief Resource and upgrade item types used across the interactive world.
 */
enum class ItemType {
  Wood,  ///< Wood resource.
  Stone, ///< Stone resource.
  Metal  ///< Metal resource.
};

/**
 * @brief Convert an ItemType value to a display string.
 * @param itemType item type to convert
 * @return string view naming the item type, or "Unknown" for unhandled values
 */
constexpr std::string_view ItemTypeToString(ItemType itemType) {
  switch (itemType) {
  case ItemType::Wood:
    return "Wood";
  case ItemType::Stone:
    return "Stone";
  case ItemType::Metal:
    return "Metal";
  }

  return "Unknown";
}

} // namespace cse498
