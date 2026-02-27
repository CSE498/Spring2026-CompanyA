/**
*  @file Text.hpp
 * @author Kiana May
 *
 * A text rendering class for styled text display in the GUI interface.
 * Disclaimer: Code clean up and comments utilized Claude Sonnet 4.5
 *
 */

#ifndef TEXT_HPP
#define TEXT_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

namespace cse498 {

/// Color structure for text rendering
/// not inside Text as other classes may use Color
struct Color {
    unsigned char r = 255;  ///Red component (0-255)
    unsigned char g = 255;  ///Green component (0-255)
    unsigned char b = 255;  ///Blue component (0-255)
    unsigned char a = 255;  ///Alpha/transparency (0-255)

    Color() = default;
    Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};


/**
 * @class Text
 * @brief Manages styled text rendering for the GUI interface.
 *
 * This class handles text properties such as font, color, size, and style (bold/italic),
 * providing a simplified interface for rendering text in SDL-based GUI applications.
 * It abstracts away the complexities of SDL_ttf font rendering.
 */

class Text {
protected:
    std::string mContent;           ///Text content to render
    std::string mFontPath;          ///Path to current font file
    Color mColor;                   ///Text color
    int mFontSize;                  ///Font size in points
    bool mBold;                     ///Bold style flag
    bool mItalic;                   ///Italic style flag

    SDL_Renderer* mRenderer;        ///SDL renderer for drawing
    TTF_Font* mFont;                ///Loaded SDL font object


    static const std::string DEFAULT_FONT;  ///Default font path
    static const int DEFAULT_SIZE = 16;     ///Default font size
    static const Color DEFAULT_COLOR;      ///Default font color (White)

    /// Reload the font with current settings (size, style)
    /// @throws std::runtime_error if font loading fails
    void ReloadFont();

    /// Get SDL font style flags based on bold/italic settings
    /// @return SDL font style flags
    int GetFontStyle() const;

public:
    /**
     * Constructor
     * @param renderer SDL renderer for drawing (can be set later)
     */
    explicit Text(SDL_Renderer* renderer = nullptr);

    /**
     * Destructor
     */
    ~Text();

    // Disable copying to avoid issues with font management
    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;

    // Allow moving
    Text(Text&& other) noexcept;
    Text& operator=(Text&& other) noexcept;

     /// Dimensions structure for text measurements
     struct Dimensions {
      int width;   ///< Width in pixels
      int height;  ///< Height in pixels
     };

    /**
     * Set the text content to display
     * @param text The string to render
     */
    void SetContent(const std::string& text);

    /**
     * Get the current text content
     * @return The current text string
     */
    const std::string& GetContent() const { return mContent; }

    /**
     * Set the text color
     * @param c Color structure with RGBA values
     */
    void SetColor(Color c);

    /**
     * Set the text color using individual RGB(A) components
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @param a Alpha/transparency (0-255, default 255)
     */
    void SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

    /**
     * Get the current text color
     * @return Current Color
     */
    const Color& GetColor() const { return mColor; }

    /**
     * Set the font by name/path
     * @param font_name Path to font file or font identifier
     * @throws std::runtime_error if font cannot be loaded
     */
    void SetFont(const std::string& font_name);

    /**
     * Get the current font path
     * @return Current font path/name
     */
    const std::string& GetFont() const { return mFontPath; }

    /**
     * Set the font size in points
     * @param point_size Font size (must be positive)
     * @note Asserts if point_size <= 0; Passing a non-positive value is a programmer error.
     */
    void SetSize(int point_size);

    /**
     * Get the current font size
     * @return Current font size in points
     */
    int GetSize() const { return mFontSize; }

    /**
     * Set bold style
     * @param bold True to enable bold, false to disable
     */
    void SetBold(bool bold);

    /**
     * Get bold status
     * @return True if bold is enabled
     */
    bool IsBold() const { return mBold; }

    /**
     * Set italic style
     * @param italic True to enable italic, false to disable
     */
    void SetItalic(bool italic);

    /**
     * Get italic status
     * @return True if italic is enabled
     */
    bool IsItalic() const { return mItalic; }

    /**
     * Reset all style properties to defaults
     * Font: default font, Size: 16, Color: white, Bold/Italic: false
     */
    void ResetStyle();

    /**
     * Draw the text at the specified position
     * @param x X coordinate (left edge of text)
     * @param y Y coordinate (top edge of text)
     * @note Returns silently if content is empty (user error)
     */
    void Draw(int x, int y);

    /**
    * Get both width and height in a single call
    * @return Struct containing width and height in pixels
    */
     Dimensions GetDimensions() const;

    /**
     * Get the width of the rendered text in pixels
     * @return Width in pixels, or 0 if content is empty
     */
    int GetWidth() const;

    /**
     * Get the height of the rendered text in pixels
     * @return Height in pixels, or 0 if content is empty
     */
    int GetHeight() const;

    /**
     * Set the SDL renderer to use for drawing
     * @param renderer Pointer to SDL_Renderer
     */
    void SetRenderer(SDL_Renderer* renderer);

    /**
     * Get the current renderer
     * @return Pointer to current SDL_Renderer
     */
    SDL_Renderer* GetRenderer() const { return mRenderer; }
};

}
#endif