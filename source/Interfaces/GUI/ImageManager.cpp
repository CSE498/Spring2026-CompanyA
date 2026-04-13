/**
 * @file ImageManager.cpp
 * @author Sitara Baxendale
 *
 * AI Disclaimer: Used ChatGPT to check overall class correctness. It helped convert the code into using
 * std::expected rather than exceptions: https://chatgpt.com/share/69c405cb-c2f8-8329-b34a-e387ea631bbd
 *
 */

#include "ImageManager.hpp"

#include <cassert>
#include <stdexcept>
#include <expected>

namespace cse498
{
    /**
     * Constructor
     * @param renderer used to create & render textures
     */
    ImageManager::ImageManager(SDL_Renderer* renderer)
        : mRenderer(renderer)
    {
        // if a renderer DNE, message will be output
        assert(mRenderer && "ImageManager requires a valid SDL_Renderer");
    }

    /**
     * Loads images & stores them with a name
     * @param name unique name of the image
     * @param file_path path to the image file
     * @return std::expected<void, std::string> empty on success, error message on failure
     */
    std::expected<void, std::string> ImageManager::LoadImage(const std::string& name,
                                                             const std::string& file_path)
    {
        // empty string checks
        if (name.empty() || file_path.empty())
        {
            // NOTE: your IDE might show a red underline for "std::unexpected" from not being up to date w/ C++23
            // it compiles and runs in the gameview executable and tests!
            return std::unexpected("Image name or file path can't be empty.");
        }

        // avoids duplications/overwrites
        if (HasImage(name))
        {
            return std::unexpected("Duplicate image name: " + name);
        }

        // load the file into memory as a surface (freed automatically via custom deleter)
        SurfacePtr surface;
        surface.reset(IMG_Load(file_path.c_str()));
        if (!surface)
        {
            // if surface could not be created, throw error
            return std::unexpected("Failed to load image: " + file_path + " (" + IMG_GetError() + ")");
        }

        // convert surface to a texture (freed automatically via custom deleter)
        TexturePtr texture;
        texture.reset(SDL_CreateTextureFromSurface(mRenderer, surface.get()));

        // if surface couldn't be converted, throw error
        if (!texture)
        {
            return std::unexpected("Failed to create texture: " + std::string(SDL_GetError()));
        }

        // add texture to map
        mTextures[name] = std::move(texture);

        // successful
        return {};
    }

    /**
     * Checks if an image is a part of the texture map
     * @param name unique name of the image
     * @return bool whether it exists or not
     */
    bool ImageManager::HasImage(const std::string& name) const
    {
        return mTextures.find(name) != mTextures.end();
    }

    /**
     * Gets the texture of a specified image name
     * @param name unique name of the image
     * @return pointer to the texture associated with the image, otherwise nullptr
     */
    SDL_Texture * ImageManager::GetTexture(const std::string& name) const
    {
        auto pair = mTextures.find(name);
        // if it DNE
        if (pair == mTextures.end())
        {
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
    bool ImageManager::DrawImage(const std::string& name, int x, int y) const
    {
        // get texture if it exists (raw ptr here b/c we're getting it from mTextures)
        SDL_Texture* texture = GetTexture(name);
        if (!texture)
        {
            return false;
        }

        // set bounds of image using a rectangle
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;

        // query the texture's width and height, storing them in rect (returns 0 on success)
        if (SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h) != 0)
            return false;
        // copy the texture to the current rendering target (returns 0 on success)
        if (SDL_RenderCopy(mRenderer, texture, nullptr, &rect) != 0)
            return false;

        return true;
    }

    // Overload version of DrawImage
    bool ImageManager::DrawImage(const std::string& name, int x, int y, int w, int h) const
    {
        auto it = mTextures.find(name);
        if (it == mTextures.end()) return false;

        SDL_Rect dest = { x, y, w, h };
        SDL_RenderCopy(mRenderer, it->second.get(), nullptr, &dest);
        return true;
    }
}