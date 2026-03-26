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

namespace cse498 {

// custom deleter for SDL_Texture to use with std::unique_ptr
struct TextureDeleter {
  void operator()(SDL_Texture *texture) const {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

class ImageManager {
private:
  // renderer for images
  SDL_Renderer *mRenderer;

  // map to store & associate images names to textures
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>>
      mTextures;

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
  void load_image(const std::string &name, const std::string &file_path);
  bool has_image(const std::string &name) const;
  bool draw_image(const std::string &name, int x, int y) const;

  // ImageManager retains ownership of this texture. Do not call
  // SDL_DestroyTexture on the returned pointer.
  SDL_Texture *get_texture(const std::string &name) const;
};

} // namespace cse498

#endif