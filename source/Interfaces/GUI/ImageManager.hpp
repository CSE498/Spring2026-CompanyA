/**
 * @file ImageManager.hpp
 * @author Sitara Baxendale
 *
 * Class that handles image assets by name, preloading them and
 * displaying them on the screen as requested
 *
 */
 
#ifndef IMAGEMANAGER_HPP
#define IMAGEMANAGER_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <unordered_map>

namespace cse498 {

    class ImageManager {
    private:
        // renderer for images
        SDL_Renderer* mRenderer;

        // map to store & associate images names to textures
        std::unordered_map<std::string, SDL_Texture*> mTextures;

    public:
        // constructor & destructor
        explicit ImageManager(SDL_Renderer* renderer);
        ~ImageManager();

        // cannot be copied
        ImageManager(const ImageManager&) = delete;
    	ImageManager& operator=(const ImageManager&) = delete;

        // class functions
        void load_image(const std::string& name, const std::string& file_path);
        bool has_image(const std::string& name) const;
        bool draw_image(const std::string& name, int x, int y) const;
        SDL_Texture* get_texture(const std::string& name) const;

    };

}

#endif