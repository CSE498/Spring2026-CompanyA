#pragma once

#include <memory>
#include <vector>

#include "ICanvasElement.hpp"

/**
 * WebCanvas
 */
class WebCanvas {
public:
    WebCanvas() = default;
    ~WebCanvas() = default;

    WebCanvas(const WebCanvas&) = delete;
    WebCanvas& operator=(const WebCanvas&) = delete;
    WebCanvas(WebCanvas&&) = default;
    WebCanvas& operator=(WebCanvas&&) = default;

    void addElement(std::unique_ptr<ICanvasElement> element);
    void clearElements();
    void renderFrame();

private:
    std::vector<std::unique_ptr<ICanvasElement>> m_elements;
};
