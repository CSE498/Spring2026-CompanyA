/**
 * @file ImageManager.hpp
 * @author Sitara Baxendale
 *
 * Class that handles image assets by name, preloading them and
 * displaying them on the screen as requested
 *
 * AI Disclaimer: Used ChatGPT to write the TextureDeleter custom deleter
 * for better memory handling
 *
 */

#ifndef IMAGEMANAGER_HPP
#define IMAGEMANAGER_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <memory>
#include <expected>

namespace cse498 {

    // custom deleter for SDL_Texture to use with std::unique_ptr
    struct TextureDeleter {
        void operator()(SDL_Texture* texture) const {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
    };

    // type alias for readability
    using TexturePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;

    // custom deleter for SDL_Surface to use with std::unique_ptr
    struct SurfaceDeleter {
        void operator()(SDL_Surface* surface) const {
            if (surface) {
                SDL_FreeSurface(surface);
            }
        }
    };

    // type alias for readability
    using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

class ImageManager {
private:
  // renderer for images
  SDL_Renderer *mRenderer;

        // map to store & associate images names to textures
        std::unordered_map<std::string, TexturePtr> mTextures;

public:
  // constructor
  explicit ImageManager(SDL_Renderer *renderer);

  // cannot be copied, default destructor & move operations
  ~ImageManager() = default;
  ImageManager(const ImageManager &) = delete;
  ImageManager &operator=(const ImageManager &) = delete;
  ImageManager(ImageManager &&) = default;
  ImageManager &operator=(ImageManager &&) = default;

        // class functions
        std::expected<void, std::string> LoadImage(const std::string& name, const std::string& file_path);
        bool HasImage(const std::string& name) const;

        /// draw image at (x, y) at its native size
        bool DrawImage(const std::string& name, int x, int y) const;

        /// draw image at (x, y) scaled to (w, h) pixels. overload function
        bool DrawImage(const std::string& name, int x, int y, int w, int h) const;

        // ImageManager retains ownership of this texture. do not call SDL_DestroyTexture on the returned pointer.
        SDL_Texture* GetTexture(const std::string& name) const;

    };

} // namespace cse498

#endif