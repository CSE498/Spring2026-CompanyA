/**
 * @file ImageGrid.cpp
 * @author Deni Tepic
 *
 */

#include "ImageGrid.hpp"
#include "ImageManager.hpp"

#include <cassert>
#include <algorithm>

using namespace cse498;

/**
 * construct a grid with dimensions and tile size.
 */
ImageGrid::ImageGrid(size_t width, size_t height,
                     size_t tile_width, size_t tile_height)
  : mWidth(width),
    mHeight(height),
    mTileWidth(tile_width),
    mTileHeight(tile_height),
    mCells(width * height, "")
{
  assert(tile_width > 0);
  assert(tile_height > 0);
}

/**
 * get image name at specified cell.
 */
const std::string& ImageGrid::GetCell(size_t x, size_t y) const {
  assert(IsValid(x, y));
  return mCells[ToIndex(x, y)];
}

/**
 * set image name at specified cell.
 */
void ImageGrid::SetCell(size_t x, size_t y,
                        const std::string& image_name)
{
  assert(IsValid(x, y));
  mCells[ToIndex(x, y)] = image_name;
}

void ImageGrid::Fill(const std::string& image_name) {
  for (auto& cell : mCells) cell = image_name;
}

/**
 * clear all image names.
 */
void ImageGrid::Clear() {
  for (auto& cell : mCells) {
    cell.clear();
  }
}

/**
 * resize grid while preserving overlapping region.
 * *AIGENERATED double for-loop*
 */
void ImageGrid::Resize(size_t new_width, size_t new_height)
{
  std::vector<std::string> new_cells(new_width * new_height, "");

  size_t min_width = std::min(mWidth, new_width);
  size_t min_height = std::min(mHeight, new_height);

  for (size_t x = 0; x < min_width; ++x) {
    for (size_t y = 0; y < min_height; ++y) {
      new_cells[x + y * new_width] =
          mCells[ToIndex(x, y)];
    }
  }

  mCells.swap(new_cells);
  mWidth = new_width;
  mHeight = new_height;
}

/**
 * draw the entire grid.
 * converts grid coordinates to pixel coordinates
 * using the configured tile dimensions.
 *  *AIGENERATED*
 */
void ImageGrid::Draw(const ImageManager& image_manager) const
{
  for (size_t y = 0; y < mHeight; ++y) {
    for (size_t x = 0; x < mWidth; ++x) {

      const std::string& image_name =
          mCells[ToIndex(x, y)];

      if (!image_name.empty()) {

        int pixel_x = static_cast<int>(x) * mTileWidth;
        int pixel_y = static_cast<int>(y) * mTileHeight;

        image_manager.draw_image(image_name, pixel_x, pixel_y,
                         static_cast<int>(mTileWidth),
                         static_cast<int>(mTileHeight));
      }
    }
  }
}

void ImageGrid::DrawViewport(const ImageManager& image_manager,
                               int cam_x, int cam_y,
                               int viewport_w, int viewport_h) const {
  int tw = static_cast<int>(mTileWidth);
  int th = static_cast<int>(mTileHeight);

  // Which tiles are visible?
  int start_x = std::max(0, cam_x);
  int start_y = std::max(0, cam_y);
  int end_x   = std::min(static_cast<int>(mWidth),  cam_x + (viewport_w / tw) + 1);
  int end_y   = std::min(static_cast<int>(mHeight), cam_y + (viewport_h / th) + 1);

  for (int y = start_y; y < end_y; ++y) {
    for (int x = start_x; x < end_x; ++x) {
      const std::string& name = mCells[ToIndex(x, y)];
      if (name.empty()) continue;
      int px = (x - cam_x) * tw;
      int py = (y - cam_y) * th;
      image_manager.draw_image(name, px, py, tw, th);
    }
  }
}

