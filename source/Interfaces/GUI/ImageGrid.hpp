/**
 * @file ImageGrid.hpp
 * @author Deni Tepic
 *
 * A grid of image names used to render the world background.
 */

#pragma once

#include <string>
#include <vector>

namespace cse498
{

    class ImageManager;

    /**
     * Represents a 2D grid of image names for rendering.
     *
     * ImageGrid stores image names in grid-space using cell coordinates.
     * It converts cell positions to pixel positions using fixed tile dimensions.
     * Rendering is delegated to ImageManager.
     */
    class ImageGrid
    {
    private:
        size_t mWidth = 0;  ///< Grid width in cells
        size_t mHeight = 0; ///< Grid height in cells

        size_t mTileWidth = 0;  ///< Tile width in pixels
        size_t mTileHeight = 0; ///< Tile height in pixels

        std::vector<std::string> mCells; ///< One image name per cell

        /**
         * Convert grid coordinates to a vector index.
         * @param x X coordinate in grid space
         * @param y Y coordinate in grid space
         * @return Index into mCells
         */
        [[nodiscard]] size_t ToIndex(size_t x, size_t y) const
        {
            return x + y * mWidth;
        }

    public:
        /**
         * Construct a grid with dimensions and tile size.
         * @param width Grid width in cells
         * @param height Grid height in cells
         * @param tile_width Tile width in pixels
         * @param tile_height Tile height in pixels
         */
        ImageGrid(size_t width, size_t height, size_t tile_width, size_t tile_height);

        /**
         * Get the grid width.
         * @return Grid width in cells
         */
        [[nodiscard]] size_t GetWidth() const { return mWidth; }

        /**
         * Get the grid height.
         * @return Grid height in cells
         */
        [[nodiscard]] size_t GetHeight() const { return mHeight; }

        /**
         * Get the tile width.
         * @return Tile width in pixels
         */
        [[nodiscard]] size_t GetTileWidth() const { return mTileWidth; }

        /**
         * Get the tile height.
         * @return Tile height in pixels / I'm sure you're understanding by now...
         */
        [[nodiscard]] size_t GetTileHeight() const { return mTileHeight; }

        /**
         * Determine if a cell coordinate is inside the grid bounds.
         * @param x X coordinate in grid space
         * @param y Y coordinate in grid space
         * @return True if the coordinate is valid
         */
        [[nodiscard]] bool IsValid(size_t x, size_t y) const
        {
            return x < mWidth && y < mHeight;
        }

        /**
         * Get the image name stored at a cell.
         * @param x X coordinate in grid space
         * @param y Y coordinate in grid space
         * @return Image name stored at the cell
         */
        [[nodiscard]] const std::string &GetCell(size_t x, size_t y) const;

        /**
         * Set the image name stored at a cell.
         * @param x X coordinate in grid space
         * @param y Y coordinate in grid space
         * @param image_name Image name to store
         */
        void SetCell(size_t x, size_t y, const std::string &image_name);

        /**
         * Fill every cell in the grid with the same image name.
         * @param image_name Image name to place in all cells
         */
        void Fill(const std::string &image_name);

        /**
         * Clear all cells in the grid.
         */
        void Clear();

        /**
         * Resize the grid while preserving overlapping cells.
         * @param new_width New grid width in cells
         * @param new_height New grid height in cells
         */
        void Resize(size_t new_width, size_t new_height);

        /**
         * Draw the entire grid using ImageManager.
         * @param image_manager Image manager used for rendering
         */
        void Draw(const ImageManager &image_manager) const;

        /**
         * Draw only the visible portion of the grid.
         * @param image_manager Image manager used for rendering
         * @param cam_x Camera top-left X position in tile coordinates
         * @param cam_y Camera top-left Y position in tile coordinates
         * @param viewport_w Viewport width in pixels
         * @param viewport_h Viewport height in pixels
         */
        void DrawViewport(const ImageManager &image_manager,
                          int cam_x, int cam_y,
                          int viewport_w, int viewport_h) const;
    };

} // namespace cse498