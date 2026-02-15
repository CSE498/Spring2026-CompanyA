#include "WebCanvas.hpp"

#include <utility>
#include <algorithm>  // std::stable_sort
#include <vector>

WebCanvas::WebCanvas(std::string id)
  : m_id(std::move(id))
{
    if (m_id.empty()) {
        m_id = "web-canvas";
    }
}

// ---- IDomElement ----
void WebCanvas::mountToLayout(WebLayout& parent, Alignment align)
{
    // Early stage: record the mount relationship only (no real DOM operations yet).
    m_parent  = &parent;
    m_align   = align;
    m_mounted = true;
}

void WebCanvas::unmount()
{
    // Early stage: clear mount state only.
    m_parent  = nullptr;
    m_mounted = false;
}

void WebCanvas::syncFromModel()
{
    // Early stage: no-op (will later sync size/style/attributes to the real <canvas>).
}

const std::string& WebCanvas::Id() const
{
    return m_id;
}

// ---- Canvas management ----
void WebCanvas::addElement(std::unique_ptr<ICanvasElement> element)
{
    if (element) {
        m_elements.push_back(std::move(element));
    }
}

void WebCanvas::clearElements()
{
    m_elements.clear();
}

void WebCanvas::renderFrame()
{
    // Build a non-owning view for scheduling without modifying ownership order.
    std::vector<ICanvasElement*> ordered;
    ordered.reserve(m_elements.size());

    for (auto& up : m_elements) {
        if (up) {
            ordered.push_back(up.get());
        }
    }

    // Stable sort by zIndex (ascending). Elements with the same zIndex keep insertion order.
    std::stable_sort(ordered.begin(), ordered.end(),
        [](const ICanvasElement* a, const ICanvasElement* b) {
            return a->zIndex() < b->zIndex();
        });

    // Draw visible elements only.
    for (auto* e : ordered) {
        if (!e->visible()) {
            continue;
        }
        e->draw(*this);
    }
}