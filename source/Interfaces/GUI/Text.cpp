/**
 * @file Text.cpp
 * @author Kiana May
 * Implementation of the Text class for styled text rendering.
 * Disclaimer: Code clean up and comments utilized Claude Sonnet 4.5
 */

#include "Text.hpp"
#include <cassert>
#include <iostream>

using namespace cse498;

const std::string Text::DEFAULT_FONT = DEFAULT_FONT_PATH;
/**
 * Constructor
 * @param renderer SDL renderer for drawing
 */
Text::Text(SDL_Renderer* renderer)
    : mContent("")
    , mFontPath(DEFAULT_FONT)
    , mColor(DEFAULT_COLOR)
    , mFontSize(DEFAULT_SIZE)
    , mBold(false)
    , mItalic(false)
    , mInitialized(false)
    , mRenderer(renderer)
    , mFont(nullptr)
{
    // Initialize SDL_ttf if not already initialized
    // Static bool ensures TTF_Init() is called only once across ALL Text objects
    // This is necessary because TTF_Init() initializes global SDL_ttf state
    static bool ttf_initialized = false;
    if (!ttf_initialized) {
        if (TTF_Init() == -1) {
            std::cerr << "Error: Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
            return;  // mInitialized remains false
        }
        ttf_initialized = true;
    }

    // Load default font
    if (!ReloadFont()) {
        std::cerr << "Warning: Failed to load default font in constructor" << std::endl;
        // mInitialized remains false, but object is still usable (can try SetFont later)
        return;
    }

    mInitialized = true;
}

/**
 * Move constructor
 */
Text::Text(Text&& other) noexcept
    : mContent(std::move(other.mContent))
    , mFontPath(std::move(other.mFontPath))
    , mColor(other.mColor)
    , mFontSize(other.mFontSize)
    , mBold(other.mBold)
    , mItalic(other.mItalic)
    , mInitialized(other.mInitialized)
    , mRenderer(other.mRenderer)
    , mFont(std::move(other.mFont))
{
    other.mRenderer = nullptr;
    other.mInitialized = false;
}

/**
 * Move assignment operator
 */
Text& Text::operator=(Text&& other) noexcept {
    if (this != &other) {
        mContent = std::move(other.mContent);
        mFontPath = std::move(other.mFontPath);
        mColor = other.mColor;
        mFontSize = other.mFontSize;
        mBold = other.mBold;
        mItalic = other.mItalic;
        mInitialized = other.mInitialized;
        mRenderer = other.mRenderer;
        mFont = std::move(other.mFont);

        other.mRenderer = nullptr;
        other.mInitialized = false;
    }
    return *this;
}

bool Text::SetFont(const std::string& font_name) {
    std::string old_path = mFontPath;
    mFontPath = font_name;

    if (!ReloadFont()) {
        // Revert to old font on failure
        mFontPath = old_path;
        ReloadFont();  // Try to restore old font
        std::cerr << "Warning: Failed to set font '" << font_name
                  << "', keeping previous font" << std::endl;
        return false;
    }

    return true;
}

bool Text::SetSize(int point_size) {
    // Validate input
    if (point_size <= 0) {
        std::cerr << "Error: Font size must be positive, got " << point_size << std::endl;
        return false;
    }

    int old_size = mFontSize;
    mFontSize = point_size;

    if (!ReloadFont()) {
        // Revert to old size on failure
        mFontSize = old_size;
        ReloadFont();  // Try to restore
        std::cerr << "Warning: Failed to set size " << point_size << std::endl;
        return false;
    }

    return true;
}

void Text::SetBold(bool bold) {
    mBold = bold;
    if (mFont) {
        TTF_SetFontStyle(mFont.get(), GetFontStyle());
    }
}

void Text::SetItalic(bool italic) {
    mItalic = italic;
    if (mFont) {
        TTF_SetFontStyle(mFont.get(), GetFontStyle());
    }
}

void Text::ResetStyle() {
    mFontPath = DEFAULT_FONT;
    mFontSize = DEFAULT_SIZE;
    mColor = DEFAULT_COLOR;
    mBold = false;
    mItalic = false;

    ReloadFont();
}

bool Text::Draw(int x, int y) {
    // Validate state
    if (mContent.empty()) {
        return true;  // Nothing to draw is not an error
    }

    if (!mRenderer) {
        std::cerr << "Error: Cannot draw text: renderer not set" << std::endl;
        return false;
    }

    if (!mFont) {
        std::cerr << "Error: Cannot draw text: font not loaded" << std::endl;
        return false;
    }

    // Convert Color to SDL_Color
    SDL_Color sdl_color = {mColor.r, mColor.g, mColor.b, mColor.a};

    // Render text to surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended(mFont.get(), mContent.c_str(), sdl_color);
    if (!surface) {
        std::cerr << "Error: Failed to render text: " << TTF_GetError() << std::endl;
        return false;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        std::cerr << "Error: Failed to create texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set up destination rectangle
    SDL_Rect dest_rect;
    dest_rect.x = x;
    dest_rect.y = y;
    dest_rect.w = surface->w;
    dest_rect.h = surface->h;

    // Copy texture to renderer
    SDL_RenderCopy(mRenderer, texture, nullptr, &dest_rect);

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    return true;
}

Text::Dimensions Text::GetDimensions() const {
    if (mContent.empty() || !mFont) {
        return {0, 0};
    }

    int width = 0, height = 0;
    TTF_SizeUTF8(mFont.get(), mContent.c_str(), &width, &height);
    return {width, height};
}

int Text::GetWidth() const {
    if (mContent.empty() || !mFont) {
        return 0;
    }

    int width = 0;
    TTF_SizeUTF8(mFont.get(), mContent.c_str(), &width, nullptr);
    return width;
}

int Text::GetHeight() const {
    if (mContent.empty() || !mFont) {
        return 0;
    }

    int height = 0;
    TTF_SizeUTF8(mFont.get(), mContent.c_str(), nullptr, &height);
    return height;
}

bool Text::ReloadFont() {
    // Reset existing font (unique_ptr handles cleanup automatically)
    mFont.reset();

    // Try to load the font
    TTF_Font* raw_font = TTF_OpenFont(mFontPath.c_str(), mFontSize);
    if (!raw_font) {
        // Try default font as fallback
        if (mFontPath != DEFAULT_FONT) {
            raw_font = TTF_OpenFont(DEFAULT_FONT.c_str(), mFontSize);
            if (raw_font) {
                mFontPath = DEFAULT_FONT;
                std::cerr << "Warning: Using default font as fallback" << std::endl;
            }
        }

        // If still no font, return false
        if (!raw_font) {
            std::cerr << "Error: Failed to load font '" << mFontPath
                      << "': " << TTF_GetError() << std::endl;
            return false;
        }
    }

    mFont.reset(raw_font);

    // Apply style
    TTF_SetFontStyle(mFont.get(), GetFontStyle());

    return true;
}

constexpr int Text::GetFontStyle() const {
    int style = TTF_STYLE_NORMAL;
    if (mBold) {
        style |= TTF_STYLE_BOLD;
    }
    if (mItalic) {
        style |= TTF_STYLE_ITALIC;
    }
    return style;
}