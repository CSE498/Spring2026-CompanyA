#pragma once

namespace cse498 {

class WebCanvas;

/**
 * ICanvasElement
 */
class ICanvasElement {
public:
    virtual ~ICanvasElement() = default;
    virtual void Draw(WebCanvas& canvas) = 0;

    // Metadata accessors for the canvas scheduler (z-index ordering, visibility filtering).
    int ZIndex() const { return m_metadata.mZIndex; }
    bool Visible() const { return m_metadata.mIsVisible; }

    void SetZIndex(int z) { m_metadata.mZIndex = z; }
    void SetVisible(bool v) { m_metadata.mIsVisible = v; }

protected:
    struct Metadata {
        int mZIndex = 0;
        bool mIsVisible = true;
    };

    Metadata m_metadata;
};

} // namespace cse498