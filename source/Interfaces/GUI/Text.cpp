/**
 * @file Text.cpp
 * @author Kiana May
 * Implementation of the Text class for styled text rendering.
 * Disclaimer: Code clean up and comments utilized Claude Sonnet 4.5
 **/

#include "Text.hpp"

#include <stdexcept>
#include <cassert>

using namespace cse498;

//defualt font path
const std::string Text::DEFAULT_FONT = "../source/Interfaces/GUI/fonts/OpenSans-Regular.ttf";


/**
 * Constructor
 * @param renderer SDL renderer for drawing
 */
Text::Text(SDL_Renderer* renderer)
    : mContent("")
    , mFontPath(DEFAULT_FONT)
    , mColor(255, 255, 255, 255)  // White
    , mFontSize(DEFAULT_SIZE)
    , mBold(false)
    , mItalic(false)
    , mRenderer(renderer)
    , mFont(nullptr)
{
    // Initialize SDL_ttf if not already initialized
    static bool ttf_initialized = false;
    if (!ttf_initialized) {
        if (TTF_Init() == -1) {
            throw std::runtime_error(std::string("Failed to initialize SDL_ttf: ") + TTF_GetError());
        }
        ttf_initialized = true;
    }

    // Load default font
    try {
        ReloadFont();
    } catch (const std::runtime_error& e) {
        // If default font fails, we'll try to load it again later
    }
}

/**
 * Destructor
 */
Text::~Text() {
    if (mFont) {
        TTF_CloseFont(mFont);
    }
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
    , mRenderer(other.mRenderer)
    , mFont(other.mFont)
{
    other.mFont = nullptr;
    other.mRenderer = nullptr;
}

/**
 * Move assignment operator
 */
Text& Text::operator=(Text&& other) noexcept {
    if (this != &other) {
        if (mFont) {
            TTF_CloseFont(mFont);
        }

        mContent = std::move(other.mContent);
        mFontPath = std::move(other.mFontPath);
        mColor = other.mColor;
        mFontSize = other.mFontSize;
        mBold = other.mBold;
        mItalic = other.mItalic;
        mRenderer = other.mRenderer;
        mFont = other.mFont;

        other.mFont = nullptr;
        other.mRenderer = nullptr;
    }
    return *this;
}

void Text::SetContent(const std::string& text) {
    mContent = text;
}

void Text::SetColor(Color c) {
    mColor = c;
}

void Text::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    mColor.r = r;
    mColor.g = g;
    mColor.b = b;
    mColor.a = a;
}

void Text::SetFont(const std::string& font_name) {
    mFontPath = font_name;
    ReloadFont();  // May throw if font doesn't exist
}

void Text::SetSize(int point_size) {
    assert(point_size > 0 && "Font size must be positive");

    if (point_size <= 0) {
        return;  // Assert should catch this in debug mode
    }

    mFontSize = point_size;
    ReloadFont();  // Reload font with new size
}

void Text::SetBold(bool b) {
    mBold = b;
    if (mFont) {
        TTF_SetFontStyle(mFont, GetFontStyle());
    }
}

void Text::SetItalic(bool i) {
    mItalic = i;
    if (mFont) {
        TTF_SetFontStyle(mFont, GetFontStyle());
    }
}

void Text::ResetStyle() {
    mFontPath = DEFAULT_FONT;
    mFontSize = DEFAULT_SIZE;
    mColor = Color(255, 255, 255, 255);  // White
    mBold = false;
    mItalic = false;

    ReloadFont();
}

void Text::Draw(int x, int y) {
    // User error: empty content - silently return
    if (mContent.empty()) {
        return;
    }

    if (!mRenderer) {
        throw std::runtime_error("Cannot draw text: renderer not set");
    }

    if (!mFont) {
        throw std::runtime_error("Cannot draw text: font not loaded");
    }

    // Convert Color to SDL_Color
    SDL_Color sdl_color = {mColor.r, mColor.g, mColor.b, mColor.a};

    // Render text to surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended(mFont, mContent.c_str(), sdl_color);
    if (!surface) {
        throw std::runtime_error(std::string("Failed to render text: ") + TTF_GetError());
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        throw std::runtime_error(std::string("Failed to create texture: ") + SDL_GetError());
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
}

int Text::GetWidth() const {
    if (mContent.empty() || !mFont) {
        return 0;
    }

    int width = 0;
    TTF_SizeUTF8(mFont, mContent.c_str(), &width, nullptr);
    return width;
}

int Text::GetHeight() const {
    if (mContent.empty() || !mFont) {
        return 0;
    }

    int height = 0;
    TTF_SizeUTF8(mFont, mContent.c_str(), nullptr, &height);
    return height;
}

void Text::SetRenderer(SDL_Renderer* r) {
    mRenderer = r;
}

void Text::ReloadFont() {
    // Close existing font if any
    if (mFont) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }

    // Try to load the font
    mFont = TTF_OpenFont(mFontPath.c_str(), mFontSize);
    if (!mFont) {
        // Try default font as fallback
        if (mFontPath != DEFAULT_FONT) {
            mFont = TTF_OpenFont(DEFAULT_FONT.c_str(), mFontSize);
            if (mFont) {
                // Successfully loaded default font, update mFontPath
                mFontPath = DEFAULT_FONT;
            }
        }

        // If still no font, throw exception
        if (!mFont) {
            throw std::runtime_error(std::string("Failed to load font '") + mFontPath +
                                   "': " + TTF_GetError());
        }
    }

    // Apply style
    TTF_SetFontStyle(mFont, GetFontStyle());
}

int Text::GetFontStyle() const {
    int style = TTF_STYLE_NORMAL;
    if (mBold) {
        style |= TTF_STYLE_BOLD;
    }
    if (mItalic) {
        style |= TTF_STYLE_ITALIC;
    }
    return style;
}