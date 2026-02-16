#pragma once

class WebCanvas;

/**
 * ICanvasElement
 */
class ICanvasElement {
public:
    virtual ~ICanvasElement() = default;
    virtual void draw(WebCanvas& canvas) = 0;

    // Metadata accessors for the canvas scheduler (z-index ordering, visibility filtering).
    int zIndex() const { return m_metadata.zIndex; }
    bool visible() const { return m_metadata.visible; }

    void setZIndex(int z) { m_metadata.zIndex = z; }
    void setVisible(bool v) { m_metadata.visible = v; }

protected:
    struct Metadata {
        int zIndex = 0;
        bool visible = true;
    };

    Metadata m_metadata;
};
