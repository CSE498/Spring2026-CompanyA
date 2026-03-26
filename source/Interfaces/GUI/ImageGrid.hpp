/**
 * @file ImageGrid.hpp
 * @author Deni Tepic
 *
 * A grid of image names used to render world background
 **/

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace cse498 {

class ImageManager; // Forward declaration

/**
 * @class ImageGrid
 * @brief Represents a 2D grid of image names for rendering.
 *
 * ImageGrid operates in grid-space (cell coordinates).
 * It converts cell positions to pixel positions using a fixed tile size.
 * Rendering is delegated to ImageManager.
 */
class ImageGrid {
private:
  size_t mWidth = 0;
  size_t mHeight = 0;

  size_t mTileWidth = 0;
  size_t mTileHeight = 0;

  std::vector<std::string> mCells; ///< One image name per cell

  /// Convert (x,y) to vector index
  [[nodiscard]] size_t ToIndex(size_t x, size_t y) const {
    return x + y * mWidth;
  }

public:
  /// Construct grid with dimensions and tile size
  ImageGrid(size_t width, size_t height, size_t tile_width, size_t tile_height);

  // -- Accessors --

  [[nodiscard]] size_t GetWidth() const { return mWidth; }
  [[nodiscard]] size_t GetHeight() const { return mHeight; }

  [[nodiscard]] size_t GetTileWidth() const { return mTileWidth; }
  [[nodiscard]] size_t GetTileHeight() const { return mTileHeight; }

  [[nodiscard]] bool IsValid(size_t x, size_t y) const {
    return x < mWidth && y < mHeight;
  }

  /// Get image name at a cell
  [[nodiscard]] const std::string &GetCell(size_t x, size_t y) const;

  /// Set image name at a cell
  void SetCell(size_t x, size_t y, const std::string &image_name);

  /// Clear all cells (set to empty string)
  void Clear();

  /// Resize grid and preserve overlapping cells
  void Resize(size_t new_width, size_t new_height);

  /// Draw entire grid using ImageManager
  void Draw(const ImageManager &image_manager) const;
};

} // namespace cse498
