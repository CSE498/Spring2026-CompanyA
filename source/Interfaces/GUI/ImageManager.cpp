/**
 * @file ImageManager.cpp
 * @author Sitara Baxendale
 *
 * AI Disclaimer: Used ChatGPT to check overall class correctness. It helped me
 * handle my memory in a safer manner & suggested to throw error messages
 * in the load_image class.
 */

#include "ImageManager.hpp"

#include <cassert>
#include <stdexcept>

namespace cse498 {
/**
 * Constructor
 * @param renderer used to create & render textures
 */
ImageManager::ImageManager(SDL_Renderer *renderer) : mRenderer(renderer) {
  // if a renderer DNE, message will be output
  assert(mRenderer && "ImageManager requires a valid SDL_Renderer");
}

/**
 * Loads images & stores them with a name
 * @param name unique name of the image
 * @param file_path path to the image file
 */
void ImageManager::load_image(const std::string &name,
                              const std::string &file_path) {
  // empty string checks
  if (name.empty() || file_path.empty()) {
    throw std::invalid_argument("Image name or file path can't be empty.");
  }

  // avoids duplications/overwrites
  if (has_image(name)) {
    throw std::runtime_error("Duplicate image name: " + name);
  }

  // load the file into memory as a surface (not texture yet because it's an
  // image)
  SDL_Surface *surface = IMG_Load(file_path.c_str());
  if (!surface) {
    // if surface could not be created, throw error
    throw std::runtime_error("Failed to load image: " + file_path + " (" +
                             IMG_GetError() + ")");
  }

  // convert surface to a texture & free it
  SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, surface);
  SDL_FreeSurface(surface);

  // if surface couldn't be converted, throw error
  if (!texture) {
    throw std::runtime_error("Failed to create texture: " +
                             std::string(SDL_GetError()));
  }

  // add texture to map
  mTextures[name].reset(texture);
}

/**
 * Checks if an image is a part of the texture map
 * @param name unique name of the image
 * @return bool whether it exists or not
 */
bool ImageManager::has_image(const std::string &name) const {
  return mTextures.find(name) != mTextures.end();
}

/**
 * Gets the texture of a specified image name
 * @param name unique name of the image
 * @return pointer to the texture associated with the image, otherwise nullptr
 */
SDL_Texture *ImageManager::get_texture(const std::string &name) const {
  auto pair = mTextures.find(name);
  // if it DNE
  if (pair == mTextures.end()) {
    return nullptr;
  }
  // texture is the value in the key-value pair
  return pair->second.get();
}

/**
 * Draws a requested image
 * @param name unique name of the image
 * @param x x-coordinate
 * @param y y-coordinate
 * @return bool whether the image was drawn or not
 */
bool ImageManager::draw_image(const std::string &name, int x, int y) const {
  // get texture if it exists
  SDL_Texture *texture = get_texture(name);
  if (!texture) {
    return false;
  }

  // set bounds of image using a rectangle
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;

  // query the attributes of the texture (returns 0 on success)
  if (SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h) != 0)
    return false;
  // copy the texture to the current rendering target (returns 0 on success)
  if (SDL_RenderCopy(mRenderer, texture, nullptr, &rect) != 0)
    return false;

  return true;
}
} // namespace cse498