/**
 * @file ImageGrid.cpp
 * @author Deni Tepic
 */


#include "ImageGrid.hpp"
#include "ImageManager.hpp"

#include <cassert>
#include <algorithm>

using namespace cse498;

/**
 * Construct a grid with dimensions and tile size.
 * @param width Grid width in cells
 * @param height Grid height in cells
 * @param tile_width Tile width in pixels
 * @param tile_height Tile height in pixels
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
 * Get the image name stored at a cell.
 * @param x X coordinate in grid space
 * @param y Y coordinate in grid space
 * @return Image name stored at the given cell
 */
const std::string& ImageGrid::GetCell(size_t x, size_t y) const
{
    assert(IsValid(x, y));
    return mCells[ToIndex(x, y)];
}

/**
 * Set the image name stored at a cell.
 * @param x X coordinate in grid space
 * @param y Y coordinate in grid space
 * @param image_name Image name to store
 */
void ImageGrid::SetCell(size_t x, size_t y, const std::string& image_name)
{
    assert(IsValid(x, y));
    mCells[ToIndex(x, y)] = image_name;
}

/**
 * Fill every cell in the grid with the same image name.
 * @param image_name Image name to place in all cells
 */
void ImageGrid::Fill(const std::string& image_name)
{
    std::fill(mCells.begin(), mCells.end(), image_name);
}

/**
 * Clear all cells in the grid.
 */
void ImageGrid::Clear()
{
    std::fill(mCells.begin(), mCells.end(), "");
}

/**
 * Resize the grid while preserving overlapping cells.
 * @param new_width New grid width in cells
 * @param new_height New grid height in cells
 */
void ImageGrid::Resize(size_t new_width, size_t new_height)
{
    std::vector<std::string> new_cells(new_width * new_height, "");

    size_t min_width = std::min(mWidth, new_width);
    size_t min_height = std::min(mHeight, new_height);

    auto newIndex = [new_width](size_t x, size_t y)
    {
        return x + y * new_width;
    };

    for (size_t y = 0; y < min_height; ++y) {
        for (size_t x = 0; x < min_width; ++x) {
            new_cells[newIndex(x, y)] = mCells[ToIndex(x, y)];
        }
    }

    mCells.swap(new_cells);
    mWidth = new_width;
    mHeight = new_height;
}

/**
 * Draw the entire grid.
 * Converts grid coordinates to pixel coordinates using the
 * configured tile dimensions.
 * @param image_manager Image manager used for rendering
 */
void ImageGrid::Draw(const ImageManager& image_manager) const
{
    auto drawCell = [&](size_t x, size_t y)
    {
        const std::string& image_name = mCells[ToIndex(x, y)];
        if (image_name.empty()) {
            return;
        }

        int pixel_x = static_cast<int>(x) * static_cast<int>(mTileWidth);
        int pixel_y = static_cast<int>(y) * static_cast<int>(mTileHeight);

        image_manager.DrawImage(image_name,
                                pixel_x,
                                pixel_y,
                                static_cast<int>(mTileWidth),
                                static_cast<int>(mTileHeight));
    };

    for (size_t y = 0; y < mHeight; ++y) {
        for (size_t x = 0; x < mWidth; ++x) {
            drawCell(x, y);
        }
    }
}

/**
 * Draw only the visible portion of the grid.
 * @param image_manager Image manager used for rendering
 * @param cam_x Camera top-left X position in tile coordinates
 * @param cam_y Camera top-left Y position in tile coordinates
 * @param viewport_w Viewport width in pixels
 * @param viewport_h Viewport height in pixels
 */
void ImageGrid::DrawViewport(const ImageManager& image_manager,
                             int cam_x, int cam_y,
                             int viewport_w, int viewport_h) const
{
    int tile_width = static_cast<int>(mTileWidth);
    int tile_height = static_cast<int>(mTileHeight);

    int start_x = std::max(0, cam_x);
    int start_y = std::max(0, cam_y);
    int end_x = std::min(static_cast<int>(mWidth), cam_x + (viewport_w / tile_width) + 1);
    int end_y = std::min(static_cast<int>(mHeight), cam_y + (viewport_h / tile_height) + 1);

    auto drawCell = [&](int x, int y)
    {
        const std::string& image_name =
            mCells[ToIndex(static_cast<size_t>(x), static_cast<size_t>(y))];

        if (image_name.empty()) {
            return;
        }

        int pixel_x = (x - cam_x) * tile_width;
        int pixel_y = (y - cam_y) * tile_height;

        image_manager.DrawImage(image_name, pixel_x, pixel_y, tile_width, tile_height);
    };

    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            drawCell(x, y);
        }
    }
}