#pragma once

class WebCanvas;

/**
 * ICanvasElement
 */
class ICanvasElement {
public:
    virtual ~ICanvasElement() = default;
    virtual void draw(WebCanvas& canvas) = 0;

protected:
    struct Metadata {
        int zIndex = 0;
        bool visible = true;
    };

    Metadata m_metadata;
};
