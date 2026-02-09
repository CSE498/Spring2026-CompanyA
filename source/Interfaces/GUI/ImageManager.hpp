/**
 * @file ImageManager.hpp
 * @author Sitara Baxendale
 *
 *
 */
 
#pragma once

#include <string>
#include <unordered_map>

namespace cse498 {

    class ImageManager {
    private:
        // map image name to file path
        std::unordered_map<std::string, std::string> mImage;

    public:
        ImageManager();
        ~ImageManager();

        void load_image(const std::string& name, const std::string& file_path);
        std::string get_image(const std::string& name);
        void draw_image(const std::string& name, int x, int y);
        bool has_image(const std::string& name);

    };

}