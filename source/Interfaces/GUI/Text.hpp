#ifndef TEXT_HPP
#define TEXT_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>
#include <optional>
#include <iostream>

namespace cse498 {

/// Color structure for text rendering
struct Color {
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;

    Color() = default;
    constexpr Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

/**
 * @class Text
 * @brief Manages styled text rendering for the GUI interface.
 */
class Text {
private:
    // Custom deleter for TTF_Font
    struct FontDeleter {
        void operator()(TTF_Font* font) const {
            if (font) {
                TTF_CloseFont(font);
            }
        }
    };

    std::string mContent;
    std::string mFontPath;
    Color mColor;
    int mFontSize;
    bool mBold;
    bool mItalic;
    bool mInitialized;  // Track if TTF_Init succeeded

    SDL_Renderer* mRenderer;
    std::unique_ptr<TTF_Font, FontDeleter> mFont;

    static const std::string DEFAULT_FONT;
    static constexpr int DEFAULT_SIZE = 16;  // constexpr for compile-time
    static constexpr Color DEFAULT_COLOR{255, 255, 255, 255};

    /// Reload the font with current settings
    /// @return true if successful, false otherwise
    bool ReloadFont();

    /// Get SDL font style flags based on bold/italic settings
    /// @return SDL font style flags
    [[nodiscard]] constexpr int GetFontStyle() const;

public:
    /// Dimensions structure for text measurements
    struct Dimensions {
        int width;
        int height;
    };

    /**
     * Constructor
     * @param renderer SDL renderer for drawing (can be nullptr)
     * @note Call IsValid() to check if initialization succeeded
     */
    explicit Text(SDL_Renderer* renderer = nullptr);

    /**
     * Destructor - automatic cleanup via unique_ptr
     */
    ~Text() = default;

    // Disable copying
    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;

    // Allow moving
    Text(Text&& other) noexcept;
    Text& operator=(Text&& other) noexcept;

    /**
     * Check if Text object is valid and ready to use
     * @return true if TTF initialized and font loaded
     */
    [[nodiscard]] bool IsValid() const { return mInitialized && mFont; }

    /**
     * Set the text content to display
     * @param text The string to render
     */
    void SetContent(const std::string& text) { mContent = text; }

    /**
     * Get the current text content
     * @return The current text string
     */
    [[nodiscard]] const std::string& GetContent() const { return mContent; }

    /**
     * Set the text color
     * @param c Color structure with RGBA values
     */
    void SetColor(Color c) { mColor = c; }

    /**
     * Set the text color using individual RGB(A) components
     */
    void SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
        mColor = Color(r, g, b, a);
    }

    /**
     * Get the current text color
     * @return Current Color
     */
    [[nodiscard]] const Color& GetColor() const { return mColor; }

    /**
     * Set the font by name/path
     * @param font_name Path to font file
     * @return true if font loaded successfully, false otherwise
     */
    bool SetFont(const std::string& font_name);

    /**
     * Get the current font path
     * @return Current font path
     */
    [[nodiscard]] const std::string& GetFont() const { return mFontPath; }

    /**
     * Set the font size in points
     * @param point_size Font size (must be positive)
     * @return true if size set successfully, false if invalid
     */
    bool SetSize(int point_size);

    /**
     * Get the current font size
     * @return Current font size in points
     */
    [[nodiscard]] int GetSize() const { return mFontSize; }

    /**
     * Set bold style
     * @param bold True to enable bold
     */
    void SetBold(bool bold);

    /**
     * Get bold status
     * @return True if bold is enabled
     */
    [[nodiscard]] bool IsBold() const { return mBold; }

    /**
     * Set italic style
     * @param italic True to enable italic
     */
    void SetItalic(bool italic);

    /**
     * Get italic status
     * @return True if italic is enabled
     */
    [[nodiscard]] bool IsItalic() const { return mItalic; }

    /**
     * Reset all style properties to defaults
     */
    void ResetStyle();

    /**
     * Draw the text at the specified position
     * @param x X coordinate (left edge)
     * @param y Y coordinate (top edge)
     * @return true if draw succeeded, false otherwise
     */
    bool Draw(int x, int y);

    /**
     * Get both width and height in a single call
     * @return Dimensions struct, or {0,0} if invalid
     */
    [[nodiscard]] Dimensions GetDimensions() const;

    /**
     * Get the width of the rendered text in pixels
     * @return Width in pixels, or 0 if invalid
     */
    [[nodiscard]] int GetWidth() const;

    /**
     * Get the height of the rendered text in pixels
     * @return Height in pixels, or 0 if invalid
     */
    [[nodiscard]] int GetHeight() const;

    /**
     * Set the SDL renderer
     * @param renderer Pointer to SDL_Renderer
     */
    void SetRenderer(SDL_Renderer* renderer) { mRenderer = renderer; }

    /**
     * Get the current renderer
     * @return Pointer to SDL_Renderer
     */
    [[nodiscard]] SDL_Renderer* GetRenderer() const { return mRenderer; }
};

} // namespace cse498

#endif