#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../internal/IDomElement.hpp"
#include "../internal/ICanvasElement.hpp"

/**
 * WebCanvas
 * - As an IDomElement: can be mounted/unmounted/synced by WebLayout (DOM lifecycle).
 * - As a canvas manager: owns ICanvasElement objects and dispatches draw() in renderFrame().
 */
class WebCanvas : public IDomElement {
public:
    explicit WebCanvas(std::string id = "web-canvas");
    ~WebCanvas() override = default;

    WebCanvas(const WebCanvas&) = delete;
    WebCanvas& operator=(const WebCanvas&) = delete;
    WebCanvas(WebCanvas&&) = default;
    WebCanvas& operator=(WebCanvas&&) = default;

    // ---- IDomElement ----
    void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
    void unmount() override;
    void syncFromModel() override;
    const std::string& Id() const override;

    // ---- Canvas content management ----
    void addElement(std::unique_ptr<ICanvasElement> element);
    void clearElements();
    void renderFrame();

private:
    std::vector<std::unique_ptr<ICanvasElement>> m_elements;

    // DOM-side state (minimal early-stage implementation)
    std::string m_id;
    WebLayout*  m_parent  = nullptr;       // Non-owning pointer to the parent layout
    Alignment   m_align   = Alignment::Start;
    bool        m_mounted = false;
};
